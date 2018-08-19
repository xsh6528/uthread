#include <uthread/executor.hpp>

namespace uthread {

thread_local Executor *Executor::this_executor_ = nullptr;

void Executor::Thread::Ref::join() {
  auto joined = joined_.lock();
  if (!joined) {
    return;
  }

  Executor::current()->sleep([&](auto thread) {
    joined->push(std::move(thread));
  });
}

Executor::Thread::Ref Executor::Thread::ref() const {
  Executor::Thread::Ref ref;
  ref.joined_ = state_->joined;
  return ref;
}

Executor::Thread::operator bool() const {
  return !!(state_);
}

Executor::Thread::Thread(std::function<void()> f) {
  state_ = std::make_unique<State>();
  state_->f = std::move(f);
  state_->stack = std::make_unique<char[]>(kStackSize);
  context_with_f(&(state_->context),
                  state_->stack.get(),
                  kStackSize,
                  thread_f,
                  nullptr);
  state_->joined = std::make_shared<std::queue<Thread>>();
}

Executor::Thread::Ref Executor::add(std::function<void()> f) {
  Thread thread(std::move(f));
  auto ref = thread.ref();
  ready_.push(std::move(thread));
  alive_++;
  return ref;
}

void Executor::ready(Executor::Thread thread) {
  DCHECK(thread);
  ready_.push(std::move(thread));
}

void Executor::run() {
  DCHECK(this_executor_ == nullptr);

  this_executor_ = this;

  while (!ready_.empty()) {
    this_thread_ = std::move(ready_.front());
    ready_.pop();
    context_swap(&executor_, &(this_thread_.state_->context));
  }

  this_thread_ = Thread();
  this_executor_ = nullptr;
}

void Executor::yield() {
  DCHECK(this_thread_);

  if (ready_.empty()) {
    return;
  }

  sleep([&](auto thread) {
    ready_.push(std::move(thread));
  });
}

size_t Executor::alive() {
  return alive_;
}

size_t Executor::ready() {
  return ready_.size();
}

Executor *Executor::current() {
  DCHECK_NOTNULL(this_executor_);

  return this_executor_;
}

void Executor::thread_f(void *_) {
  DCHECK_NOTNULL(this_executor_);
  DCHECK(this_executor_->this_thread_);

  try {
    this_executor_->this_thread_.state_->f();
  } catch (std::exception &ex) {
    LOG(ERROR) << "An exception has occurred: " << ex.what();
    std::terminate();
  }


  auto joined = this_executor_->this_thread_.state_->joined.get();

  while (joined->size() > 0) {
    this_executor_->ready(std::move(joined->front()));
    joined->pop();
  }

  this_executor_->alive_--;
  context_set(&(this_executor_->executor_));
}

}

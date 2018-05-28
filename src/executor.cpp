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

Executor *Executor::current() {
  DCHECK_NOTNULL(this_executor_);

  return this_executor_;
}

void Executor::thread_f(void *_) {
  DCHECK_NOTNULL(this_executor_);
  DCHECK(this_executor_->this_thread_);

  this_executor_->this_thread_.state_->f();
  this_executor_->alive_--;
  context_set(&(this_executor_->executor_));
}

}

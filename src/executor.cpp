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
  ref.joined_ = joined_;
  return ref;
}

Executor::Thread::operator bool() const {
  return !!(f_);
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
    context_swap(&executor_, &(this_thread_.context_));
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

Executor *Executor::current() {
  DCHECK_NOTNULL(this_executor_);

  return this_executor_;
}

void Executor::thread_f(void *_) {
  DCHECK_NOTNULL(this_executor_);
  DCHECK(this_executor_->this_thread_);

  this_executor_->this_thread_.f_();
  context_set(&(this_executor_->executor_));
}

}

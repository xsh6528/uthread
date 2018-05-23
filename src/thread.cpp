#include <glog/logging.h>

#include <uthread/thread.hpp>

namespace uthread {

std::unique_ptr<Thread> Thread::create(std::function<void()> f,
                                       size_t stack_size) {
  std::unique_ptr<Thread> thread(new Thread());
  thread->f_ = std::move(f);
  thread->stack_ = std::make_unique<char[]>(stack_size);
  context_with_f(&(thread->context_),
                 thread->stack_.get(),
                 stack_size,
                 thread_f,
                 thread.get());
  return thread;
}

void Thread::run() {
  DCHECK(status_ == Status::Waiting);

  Context current;
  next_ = &current;
  status_ = Status::Running;
  context_swap(&current, &context_);
}

void Thread::swap(Thread *other) {
  DCHECK_NOTNULL(other);
  DCHECK_NOTNULL(next_);
  DCHECK_NE(this, other);
  DCHECK(status_ == Status::Running);
  DCHECK(other->status_ == Status::Waiting);

  status_ = Status::Waiting;
  other->next_ = next_;
  other->status_ = Status::Running;
  context_swap(&context_, &(other->context_));
}

Thread::Status Thread::status() const {
  return status_;
}

void Thread::thread_f(void *arg) {
  Thread *thread = reinterpret_cast<Thread *>(arg);
  thread->f_();
  thread->status_ = Status::Finished;
  context_swap(&(thread->context_), thread->next_);
  CHECK(false) << "Unreachable!";
}

}

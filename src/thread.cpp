#include <glog/logging.h>

#include <uthread/thread.hpp>

namespace uthread {

Thread::Thread(std::function<void()> f, size_t stack_size) {
  context = std::make_unique<TContext>();
  context->f = std::move(f);
  context->stack = std::make_unique<char[]>(stack_size);
  context::with_f(&(context->context),
                  context->stack.get(),
                  stack_size,
                  thread_f,
                  context.get());
}

void Thread::run() {
  DCHECK(status() == Status::Waiting);

  context::Context current;
  context->next = &current;
  context->status = Status::Running;
  context::swap(&current, &(context->context));
}

void Thread::swap(Thread const *other) {
  DCHECK_NOTNULL(context->next);
  DCHECK_NOTNULL(other);
  DCHECK_NE(this, other);
  DCHECK(status() == Status::Running);
  DCHECK(other->status() == Status::Waiting);

  other->context->next = context->next;
  context->status = Status::Waiting;
  other->context->status = Status::Running;
  context::swap(&(context->context), &(other->context->context));
}

Thread::Status Thread::status() const {
  return context->status;
}

void Thread::thread_f(void *arg) {
  TContext *context = reinterpret_cast<TContext *>(arg);
  context->f();
  context->status = Status::Finished;
  context::swap(&(context->context), context->next);
  CHECK(false);
}

}

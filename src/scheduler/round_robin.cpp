#include <glog/logging.h>

#include <uthread/scheduler/round_robin.hpp>

namespace uthread {

RoundRobin::RoundRobin(size_t stack_size): stack_size(stack_size) {
}

void RoundRobin::spawn(std::function<void()> f) {
  auto thread = std::make_unique<Thread>();
  thread->f = std::move(f);
  thread->stack = std::make_unique<char[]>(stack_size);
  context::with_f(&(thread->context),
                  thread->stack.get(),
                  stack_size,
                  thread_f,
                  thread.get());
  thread_queue.push(std::move(thread));
}

void RoundRobin::yield() {
  DCHECK(this_thread);

  if (thread_queue.empty()) {
    return;
  }

  auto prev_thread = this_thread.get();

  this_thread = std::move(thread_queue.front());
  thread_queue.pop();

  this_thread->next = prev_thread->next;
  context::swap(&(prev_thread->context), &(this_thread->context));
}

void RoundRobin::run() {
  CHECK(!this_thread);

  context::Context current;

  while (!thread_queue.empty()) {
    this_thread = std::move(thread_queue.front());
    DCHECK(this_thread);

    thread_queue.pop();
    this_thread->next = &current;
    context::swap(&current, &(this_thread->context));

    DCHECK(this_thread);
    this_thread.release();
  }
}

void RoundRobin::thread_f(void *arg) {
  Thread *thread = reinterpret_cast<Thread *>(arg);
  thread->f();
  context::swap(&(thread->context), thread->next);
}

}

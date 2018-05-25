#include <glog/logging.h>

#include <uthread/thread.hpp>

namespace uthread {

static constexpr size_t kStackSize = 65536;

static constexpr char kNoThreadError[] =
  "This function requires a running thread.";

static thread_local std::unique_ptr<Thread> this_thread;

void Thread::spawn(std::function<void()> f) {
  CHECK(this_thread) << kNoThreadError;

  auto thread = create(f);
  thread->runner_ = this_thread->runner_;
  thread->ready_queue_ = this_thread->ready_queue_;
  this_thread->ready_queue_->push(std::move(thread));
}

void Thread::yield() {
  CHECK(this_thread) << kNoThreadError;

  if (this_thread->ready_queue_->empty()) {
    return;
  }

  swap(this_thread->ready_queue_, Status::Ready);
}

void Thread::sleep(Queue *queue) {
  CHECK(this_thread) << kNoThreadError;
  CHECK(!this_thread->ready_queue_->empty()) << "No ready threads, deadlock!";
  CHECK_NOTNULL(queue);

  swap(queue, Status::Sleeping);
}

void Thread::ready(std::unique_ptr<Thread> thread) {
  CHECK(this_thread) << kNoThreadError;
  CHECK(thread);
  CHECK(thread->status_ == Status::Sleeping);

  thread->status_ = Status::Ready;
  this_thread->ready_queue_->push(std::move(thread));
}

void Thread::run(std::function<void()> f) {
  Context runner;
  Queue ready_queue;

  auto thread = create(std::move(f));
  thread->status_ = Status::Running;
  thread->runner_ = &runner;
  thread->ready_queue_ = &ready_queue;
  ready_queue.push(std::move(thread));

  while (!ready_queue.empty()) {
    CHECK(!this_thread);
    this_thread = std::move(ready_queue.front());
    this_thread->status_ = Status::Running;
    ready_queue.pop();
    context_swap(&runner, &(this_thread->context_));
    this_thread.reset();
  }

  this_thread.reset();
}

std::unique_ptr<Thread> Thread::create(std::function<void()> f) {
  std::unique_ptr<Thread> thread(new Thread());
  thread->f_ = std::move(f);
  thread->stack_ = std::make_unique<char[]>(kStackSize);
  context_with_f(&(thread->context_),
                 thread->stack_.get(),
                 kStackSize,
                 thread_f,
                 thread.get());
  return thread;
}

void Thread::swap(Queue *queue, Status this_status) {
  DCHECK(this_thread) << kNoThreadError;
  DCHECK(!this_thread->ready_queue_->empty());
  DCHECK_NOTNULL(queue);

  auto curr_thread = this_thread.get();
  curr_thread->status_ = this_status;
  queue->push(std::move(this_thread));

  this_thread = std::move(curr_thread->ready_queue_->front());
  this_thread->status_ = Status::Running;
  this_thread->ready_queue_->pop();

  context_swap(&(curr_thread->context_), &(this_thread->context_));
}

void Thread::thread_f(void *_) {
  CHECK(this_thread);
  this_thread->f_();
  auto runner = this_thread->runner_;
  context_set(runner);
  CHECK(false) << "Unreachable!";
}

}

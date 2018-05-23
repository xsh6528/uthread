#include <glog/logging.h>

#include <uthread/worker.hpp>

namespace uthread {

void Worker::schedule(std::unique_ptr<Thread> thread) {
  DCHECK(thread);
  queue_.emplace(std::move(thread));
}

void Worker::yield() {
  DCHECK(this_thread_);

  if (queue_.empty()) {
    return;
  }

  auto curr_thread = this_thread_.get();
  queue_.push(std::move(this_thread_));

  this_thread_ = std::move(queue_.front());
  queue_.pop();

  curr_thread->swap(this_thread_.get());
}

void Worker::loop() {
  CHECK(!this_thread_);

  while (!queue_.empty()) {
    this_thread_ = std::move(queue_.front());
    queue_.pop();
    this_thread_->run();
  }

  this_thread_.reset();
}

}

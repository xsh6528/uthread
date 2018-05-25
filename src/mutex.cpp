#include <glog/logging.h>

#include <uthread/mutex.hpp>

namespace uthread {

void Mutex::acquire() {
  if (state_ == State::Locked) {
    Thread::sleep(&queue_);
  } else {
    state_ = State::Locked;
  }
}

void Mutex::release() {
  CHECK(state_ == State::Locked);

  if (!queue_.empty()) {
    Thread::ready(std::move(queue_.front()));
    queue_.pop();
  } else {
    state_ = State::Free;
  }
}

Lock::Lock(Mutex *mutex): mutex_(mutex) {
  CHECK_NOTNULL(mutex);
  mutex_->acquire();
}

Lock::~Lock() {
  mutex_->release();
}

}

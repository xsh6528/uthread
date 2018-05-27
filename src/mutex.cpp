#include <glog/logging.h>

#include <uthread/mutex.hpp>

namespace uthread {

void Mutex::acquire() {
  if (state_ == State::Locked) {
    Executor::current()->sleep([&](auto thread) {
      queue_.push(std::move(thread));
    });
  } else {
    state_ = State::Locked;
  }
}

void Mutex::release() {
  CHECK(state_ == State::Locked);

  if (!queue_.empty()) {
    Executor::current()->add(std::move(queue_.front()));
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

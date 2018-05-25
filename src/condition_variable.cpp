#include <glog/logging.h>

#include <uthread/condition_variable.hpp>

namespace uthread {

void ConditionVariable::sleep(Lock *guard) {
  CHECK_NOTNULL(guard);

  guard->sleep(&queue_);
}

void ConditionVariable::wake_one() {
  if (!queue_.empty()) {
    Thread::ready(std::move(queue_.front()));
    queue_.pop();
  }
}

void ConditionVariable::wake_all() {
  while (!queue_.empty()) {
    Thread::ready(std::move(queue_.front()));
    queue_.pop();
  }
}

}

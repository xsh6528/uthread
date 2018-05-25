#include <uthread/mutex.hpp>
#include <uthread/thread.hpp>

#ifndef UTHREAD_CONDITION_VARIABLE_HPP_
#define UTHREAD_CONDITION_VARIABLE_HPP_

namespace uthread {

/**
 * A condition variable for user threads, see the C++ docs for more info.
 *
 * https://en.cppreference.com/w/cpp/thread/condition_variable
 */
class ConditionVariable {
 public:
  /**
   * Sleeps the current thread until it is woken up.
   */
  void sleep(Lock *guard);

  /**
   * Wakes up at most one sleeping thread.
   */
  void wake_one();

    /**
   * Wakes up ALL sleeping threads.
   */
  void wake_all();

 private:
  Thread::Queue queue_;
};

}

#endif  // UTHREAD_CONDITION_VARIABLE_HPP_

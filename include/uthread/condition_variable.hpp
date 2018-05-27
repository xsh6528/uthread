#include <uthread/executor.hpp>
#include <uthread/mutex.hpp>

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
  std::queue<Executor::Thread> queue_;
};

}

#endif  // UTHREAD_CONDITION_VARIABLE_HPP_

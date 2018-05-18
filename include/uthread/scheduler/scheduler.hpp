#include <uthread/thread.hpp>

#ifndef UTHREAD_SCHEDULER_SCHEDULER_HPP_
#define UTHREAD_SCHEDULER_SCHEDULER_HPP_

namespace uthread {

class Scheduler {
 public:
  /**
   * Schedules a thread for execution.
   */
  virtual void schedule(Thread thread) = 0;

  /**
   * Yields execution of the current thread to another.
   *
   * Returns immediately if no other thread is ready to run.
   */
  virtual void yield() = 0;

  /**
   * Runs all spawned threads.
   */
  virtual void run() = 0;
};

}

#endif  // UTHREAD_SCHEDULER_SCHEDULER_HPP_

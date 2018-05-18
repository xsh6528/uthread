#include <functional>

#ifndef UTHREAD_SCHEDULER_SCHEDULER_HPP_
#define UTHREAD_SCHEDULER_SCHEDULER_HPP_

namespace uthread {

class Scheduler {
 public:
  /**
   * Spawns and schedules a thread that runs function f.
   *
   * This will eventually return a thread handle for joining threads.
   */
  virtual void spawn(std::function<void()> f) = 0;

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

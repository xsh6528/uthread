#include <queue>

#include <uthread/thread.hpp>

#ifndef UTHREAD_WORKER_HPP_
#define UTHREAD_WORKER_HPP_

namespace uthread {

/**
 * A worker that executes and switches among threads.
 */
class Worker {
 public:
  /**
   * Schedules a thread for execution.
   */
  void schedule(std::unique_ptr<Thread> thread);

  /**
   * Yields execution of the current thread to another.
   */
  void yield();

  /**
   * Runs all scheduled threads until none are left.
   */
  void loop();

 private:
  std::queue<std::unique_ptr<Thread>> queue_;

  std::unique_ptr<Thread> this_thread_;
};

}

#endif  // UTHREAD_WORKER_HPP_

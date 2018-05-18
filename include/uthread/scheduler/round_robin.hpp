#include <memory>
#include <queue>

#include <uthread/context.hpp>
#include <uthread/scheduler/scheduler.hpp>

#ifndef UTHREAD_SCHEDULER_ROUND_ROBIN_HPP_
#define UTHREAD_SCHEDULER_ROUND_ROBIN_HPP_

namespace uthread {

class RoundRobin : public Scheduler {
 public:
  /**
   * Creates a scheduler with a specified stack size for each thread.
   */
  explicit RoundRobin(size_t stack_size);

  virtual void spawn(std::function<void()> f);

  virtual void yield();

  virtual void run();

 private:
  struct Thread {
    std::function<void()> f;

    std::unique_ptr<char[]> stack;

    context::Context context;

    context::Context *next;
  };

  static void thread_f(void *arg);

  size_t stack_size;

  std::queue<std::unique_ptr<Thread>> thread_queue;

  std::unique_ptr<Thread> this_thread;
};

}

#endif  // UTHREAD_SCHEDULER_ROUND_ROBIN_HPP_

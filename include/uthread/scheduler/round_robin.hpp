#include <memory>
#include <queue>

#include <uthread/context.hpp>
#include <uthread/scheduler/scheduler.hpp>

#ifndef UTHREAD_SCHEDULER_ROUND_ROBIN_HPP_
#define UTHREAD_SCHEDULER_ROUND_ROBIN_HPP_

namespace uthread {

class RoundRobin : public Scheduler {
 public:
  virtual void schedule(Thread thread);

  virtual void yield();

  virtual void run();

 private:
  std::queue<std::unique_ptr<Thread>> queue;

  std::unique_ptr<Thread> this_thread;
};

}

#endif  // UTHREAD_SCHEDULER_ROUND_ROBIN_HPP_

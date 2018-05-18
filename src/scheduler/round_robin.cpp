#include <glog/logging.h>

#include <uthread/scheduler/round_robin.hpp>

namespace uthread {

void RoundRobin::schedule(Thread thread) {
  std::unique_ptr<Thread> thread_ptr(new Thread(std::move(thread)));
  queue.emplace(std::move(thread_ptr));
}

void RoundRobin::yield() {
  DCHECK(this_thread);

  if (queue.empty()) {
    return;
  }

  auto curr_thread = this_thread.get();
  queue.push(std::move(this_thread));

  this_thread = std::move(queue.front());
  queue.pop();

  curr_thread->swap(this_thread.get());
}

void RoundRobin::run() {
  CHECK(!this_thread);

  while (!queue.empty()) {
    this_thread = std::move(queue.front());
    queue.pop();
    this_thread->run();
  }
}

}

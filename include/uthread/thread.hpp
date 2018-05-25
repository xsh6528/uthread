#include <cstddef>
#include <functional>
#include <memory>
#include <queue>

#include <uthread/context.hpp>

#ifndef UTHREAD_THREAD_HPP_
#define UTHREAD_THREAD_HPP_

namespace uthread {

/**
 * A thread with it's own stack frame and execution context.
 */
class Thread {
 public:
  using Queue = std::queue<std::unique_ptr<Thread>>;

  /**
   * The current execution status of a thread.
   */
  enum class Status {
    /**
     * A thread that is ready to execute.
     */
    Ready,
    /**
     * A thread that is blocked from execution.
     */
    Sleeping,
    /**
     * A thread that is currently executing.
     */
    Running,
    /**
     * A thread that is finished executing.
     */
    Finished,
  };

  Thread(const Thread&) = delete;

  Thread(Thread&&) = delete;

  Thread& operator=(const Thread&) = delete;

  Thread& operator=(Thread&&) = delete;

  /**
   * Creates and schedules a thread that executes function f.
   */
  static void spawn(std::function<void()> f);

  /**
   * Context switches to another thread.
   *
   * This function returns immediately if no other threads are ready.
   */
  static void yield();

  /**
   * Sleeps the current thread and context switches to another thread.
   *
   * This function causes a fatal error if no other threads are ready. The
   * current thread is pushed onto the queue and can be woken via ready(..).
   */
  static void sleep(Queue *queue);

  /**
   * Pushes a thread onto the ready queue.
   */
  static void ready(std::unique_ptr<Thread> thread);

  /**
   * Spawns a main thread which executes function f.
   */
  static void run(std::function<void()> f);

 private:
  Thread() = default;

  static std::unique_ptr<Thread> create(std::function<void()> f);

  static void swap(Queue *queue, Status this_status);

  static void thread_f(void *arg);

  std::function<void()> f_;

  std::unique_ptr<char[]> stack_;

  Status status_ = Status::Ready;

  Context context_;

  Context const *runner_ = nullptr;

  Queue *ready_queue_ = nullptr;
};

}

#endif  // UTHREAD_THREAD_HPP_

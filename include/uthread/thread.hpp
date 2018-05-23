#include <cstddef>
#include <functional>
#include <memory>

#include <uthread/context.hpp>

#ifndef UTHREAD_THREAD_HPP_
#define UTHREAD_THREAD_HPP_

namespace uthread {

/**
 * A user thread/fiber with it's own stack frame and execution context.
 */
class Thread {
 public:
  /**
   * The current execution status of a thread.
   */
  enum class Status {
    /**
     * A thread that is ready to execute.
     */
    Waiting,
    /**
     * A thread that is currently executing.
     */
    Running,
    /**
     * A thread that is finished executing.
     */
    Finished,
  };

  /**
   * 64kb default stack.
   */
  static constexpr size_t DEFAULT_STACK_SIZE = 65536;

  /**
   * Creates a thread that executes function f when ran.
   */
  static std::unique_ptr<Thread> create(std::function<void()> f,
                                        size_t stack_size = DEFAULT_STACK_SIZE);

  Thread(const Thread&) = delete;

  Thread(Thread&&) = delete;

  Thread& operator=(const Thread&) = delete;

  Thread& operator=(Thread&&) = delete;

  /**
   * Runs the thread, returning when a thread in the execution chain finishes.
   *
   * The execution chain refers to any threads that execute as a result of this
   * thread. In other words, the thread may swap into other threads and
   * whichever one finishes executing will trigger a return.
   */
  void run();

  /**
   * Saves the execution context into this thread and switches to another.
   */
  void swap(Thread *other);

  /**
   * Returns the execution status of the thread.
   */
  Status status() const;

 private:
  Thread() = default;

  static void thread_f(void *arg);

  std::function<void()> f_;

  std::unique_ptr<char[]> stack_;

  Context context_;

  Context const *next_ = nullptr;

  Status status_ = Status::Waiting;
};

}

#endif  // UTHREAD_THREAD_HPP_

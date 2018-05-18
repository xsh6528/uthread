#include <cstddef>
#include <functional>
#include <memory>

#include <uthread/context.hpp>

#ifndef UTHREAD_THREAD_HPP_
#define UTHREAD_THREAD_HPP_

namespace uthread {

class Thread {
 public:
  enum class Status {
    /**
     * A thread that can be ran or switched into. In other words, it has not
     * finished executing and is not currently executing.
     */
    Waiting,
    /**
     * A thread that is currently executing.
     */
    Running,
    /**
     * A thread that has finished executing.
     */
    Finished,
  };

  /**
   * 64kb default stack.
   */
  static constexpr size_t DEFAULT_STACK_SIZE = 65536;

  /**
   * Creates a thread that executes function f.
   */
  explicit Thread(std::function<void()> f,
                  size_t stack_size = DEFAULT_STACK_SIZE);

  /**
   * Runs the thread.
   *
   * This function returns when the thread or another thread that was swapped
   * into from this thread finishes executing.
   */
  void run();

  /**
   * Saves the current execution context to the thread and runs another.
   */
  void swap(Thread const *other);

  /**
   * Returns the execution status of the thread.
   */
  Status status() const;

 private:
  struct TContext {
    std::function<void()> f;

    std::unique_ptr<char[]> stack;

    context::Context context;

    context::Context *next = nullptr;

    Status status = Status::Waiting;
  };

  static void thread_f(void *arg);

  std::unique_ptr<TContext> context;
};

}

#endif  // UTHREAD_THREAD_HPP_

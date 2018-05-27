#include <cstddef>
#include <functional>
#include <memory>
#include <queue>

#include <glog/logging.h>

#include <uthread/context.hpp>

#ifndef UTHREAD_EXECUTOR_HPP_
#define UTHREAD_EXECUTOR_HPP_

namespace uthread {

/**
 * A service for multiplexing a set of user threads on top of a kernel thread.
 */
class Executor {
 public:
  /**
   * A user thread with it's own stack and execution context.
   */
  class Thread {
   public:
    /**
     * A reference to a user thread, primarily for joining.
     */
    class Ref {
     public:
      /**
       * Sleeps the current executing user thread until this one finishes.
       *
       * This function returns immediately if the user thread has finished.
       */
      void join();

     private:
      std::weak_ptr<std::queue<Thread>> joined_;

      friend Thread;
    };

    static constexpr size_t kStackSize = 64 * 1024;

    /**
     * Creates a user thread that executes function f.
     */
    template<typename F>
    explicit Thread(F f): f_(std::move(f)) {
      stack_ = std::make_unique<char[]>(kStackSize);
      context_with_f(&context_,
                     stack_.get(),
                     kStackSize,
                     thread_f,
                     nullptr);
      joined_ = std::make_shared<std::queue<Thread>>();
    }

    /**
     * Creates an empty, non-executable thread.
     */
    Thread() = default;

    Thread(const Thread&) = delete;

    Thread(Thread&&) = default;

    Thread& operator=(const Thread&) = delete;

    Thread& operator=(Thread&&) = default;

    /**
     * Returns a reference to the user thread.
     */
    Ref ref() const;

    /**
     * Checks if the thread is executable.
     */
    operator bool() const;

   private:
    std::function<void()> f_;

    std::unique_ptr<char[]> stack_;

    Context context_;

    std::shared_ptr<std::queue<Thread>> joined_;

    friend Executor;
  };

  /**
   * Adds a user thread which executes function f.
   */
  template<typename F>
  Thread::Ref add(F f) {
    Thread thread(std::move(f));
    auto ref = thread.ref();
    ready_.push(std::move(thread));
    return ref;
  }

  /**
   * Pushes a user thread onto the ready queue for future execution.
   */
  void ready(Thread thread);

  /**
   * Runs all added user threads.
   */
  void run();

  /**
   * Context switches to another user threads.
   *
   * This function returns immediately if no other user threads are ready.
   */
  void yield();

  /**
   * Sleeps the current executing user thread and context switches to another.
   *
   * This function transfers ownership of the current executing user thread to
   * the function f. A fatal error occurs if no other user threads are ready.
   */
  template<typename F>
  void sleep(F f)  {
    CHECK(!ready_.empty()) << "No ready threads... deadlock!";

    if (context_get(&(this_thread_.context_)) == Snapshot::SNAPSHOT) {
      f(std::move(this_thread_));
      this_thread_ = std::move(ready_.front());
      ready_.pop();
      context_set(&(this_thread_.context_));
    }
  }

  /**
   * Returns the current executor.
   *
   * This function NEVER returns a nullptr.
   */
  static Executor *current();

 private:
  static void thread_f(void *_);

  Thread this_thread_;

  std::queue<Thread> ready_;

  Context executor_;

  static thread_local Executor *this_executor_;
};

}

#endif  // UTHREAD_EXECUTOR_HPP_

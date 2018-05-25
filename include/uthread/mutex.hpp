#include <uthread/thread.hpp>

#ifndef UTHREAD_MUTEX_HPP_
#define UTHREAD_MUTEX_HPP_

namespace uthread {

/**
 * A mutex for ensuring exclusive access between user threads.
 */
class Mutex {
 public:
  /**
   * Acquires exclusive ownership of the mutex.
   *
   * There are several cases to consider...
   *
   * 1. If the mutex is not owned by any threads, the current thread acquires
   *    the mutex.
   * 2. If the mutex is owned by another thread, the current thread blocks and
   *    will be allowed through in a future release operation.
   * 3. If the mutex is owned by the calling thread, the behavior is undefined.
   */
  void acquire();

  /**
   * Releases ownership of the mutex.
   *
   * Behavior is undefined if the calling thread does not own the mutex.
   */
  void release();

 private:
  enum class State {
    Locked,
    Free,
  };

  Thread::Queue queue_;

  State state_;
};

/**
 * An RAII guard for releasing a mutex on destruction.
 */
class Lock {
 public:
  explicit Lock(Mutex *mutex);

  Lock(const Lock &) = delete;

  Lock(Lock &&) = delete;

  Lock &operator=(const Lock &) = delete;

  Lock &operator=(Lock &&) = delete;

  ~Lock();

 private:
  Mutex *mutex_ = nullptr;
};

}

#endif  // UTHREAD_MUTEX_HPP_

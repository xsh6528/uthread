#include <queue>

#include <uthread/condition_variable.hpp>
#include <uthread/mutex.hpp>

#ifndef UTHREAD_QUEUE_HPP_
#define UTHREAD_QUEUE_HPP_

namespace uthread {

/**
 * A multi-producer multi-consumer queue of T's.
 */
template<typename T>
class MpmcQueue {
 public:
  /**
   * Creates a queue with unbounded capacity.
   */
  MpmcQueue(): MpmcQueue(0) {}

  /**
   * Creates a queue with the specified maximum capacity.
   *
   * Specifying a size of 0 indicates unbounded capacity.
   */
  explicit MpmcQueue(size_t size): size_(size) {}

  /**
   * Pops an element from the queue.
   *
   * The current thread goes to sleep if the queue is empty until an element
   * becomes available.
   */
  void pop(T &value) {
    Lock guard(&mutex_);
    while (queue_.empty()) {
      has_item_cv_.sleep(&guard);
    }
    value = std::move(queue_.front());
    queue_.pop();
    has_space_cv_.wake_one();
  }

  /**
   * Pushes an element onto the queue.
   *
   * The current thread sleeps if the queue is at capacity until another thread
   * pops an element.
   */
  void push(T value) {
    Lock guard(&mutex_);
    while (size_ != 0 && queue_.size() == size_) {
      has_space_cv_.sleep(&guard);
    }
    queue_.push(std::move(value));
    has_item_cv_.wake_one();
  }

 private:
  size_t size_;

  std::queue<T> queue_;

  ConditionVariable has_space_cv_;

  ConditionVariable has_item_cv_;

  Mutex mutex_;
};

}

#endif  // UTHREAD_QUEUE_HPP_

#include <chrono>

#include <event2/event.h>

#include <uthread/executor.hpp>

#ifndef UTHREAD_IO_HPP_
#define UTHREAD_IO_HPP_

namespace uthread {

#define UTHREAD_IO_EVENT_READABLE(EVENT)    \
(((EVENT) == ::uthread::Io::Event::Read) || \
  ((EVENT) == ::uthread::Io::Event::ReadWrite))

#define UTHREAD_IO_EVENT_WRITABLE(EVENT)    \
(((EVENT) == ::uthread::Io::Event::Write) || \
  ((EVENT) == ::uthread::Io::Event::ReadWrite))

/**
 * A service for sleeping threads on file descriptor events, timers, etc.
 */
class Io {
 public:
  enum class Event {
    Read,
    Write,
    ReadWrite,
  };

  Io();

  ~Io();

  /**
   * Sleeps the current executing thread.
   *
   * The thread is woken back up when the file discriptor is ready for reading
   * or writing, depending on the event.
   */
  Event sleep_on_fd(int fd, Event event);

  /**
   * Sleeps the current executing thread.
   *
   * The thread is woken back up when at least duration time passes. The thread
   * may sleep longer than the specified duration depending on scheduling, in
   * particular if other threads yield to the IO thread often enough.
   */
  template<typename R, typename P>
  void sleep_for(std::chrono::duration<R, P> duration) {
    std::chrono::seconds sec =
      std::chrono::duration_cast<std::chrono::seconds>(duration);

    timeval timeout;
    timeout.tv_sec  = sec.count();
    timeout.tv_usec =
      std::chrono::duration_cast<std::chrono::microseconds>(duration - sec)
        .count();

    sleep(-1, EV_TIMEOUT, &timeout);
  }

  /**
   * Adds a background thread for waking up sleeping threads.
   */
  void add(Executor *executor);

  /**
   * Returns the current IO service.
   *
   * This function NEVER returns a nullptr.
   */
  static Io *current();

 private:
  Event sleep(int fd, short eventlib_event, const timeval *timeout);  // NOLINT

  event_base *base = nullptr;

  static thread_local Io *this_io_;
};

}

#endif  // UTHREAD_IO_HPP_

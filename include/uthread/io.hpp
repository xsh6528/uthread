#include <event2/event.h>

#include <uthread/executor.hpp>

#ifndef UTHREAD_IO_HPP_
#define UTHREAD_IO_HPP_

namespace uthread {

/**
 * A service for sleeping threads waiting on IO.
 */
class Io {
 public:
  enum class Event {
    Read,
    Write,
  };

  Io();

  ~Io();

  /**
   * Sleeps the current executing thread.
   *
   * The thread is woken back up when the file discriptor is ready for reading
   * or writing, depending on the event.
   */
  void sleep(int fd, Event event);

  /**
   * Adds a background thread for IO event dispatching.
   */
  void add(Executor *executor);

 private:
  event_base *base = nullptr;
};

}

#endif  // UTHREAD_IO_HPP_

#include <event2/event.h>

#ifndef UTHREAD_LIBEVENT_HPP_
#define UTHREAD_LIBEVENT_HPP_

namespace uthread {

/**
 * An RAII wrapper around an event base.
 */
class LibeventBase {
 public:
  /**
   * Creates and configures an event base.
   */
  LibeventBase();

  ~LibeventBase();

  /**
   * Returns a pointer to the event base.
   */
  event_base* raw();

 private:
  event_base* base_ = nullptr;
};

}

#endif  // UTHREAD_LIBEVENT_HPP_

#include <glog/logging.h>

#include <uthread/io.hpp>

namespace uthread {

static void event_cb(evutil_socket_t, short, void *arg) {  // NOLINT
  Executor::Thread *thread = reinterpret_cast<Executor::Thread *>(arg);
  Executor::current()->ready(std::move(*thread));
}

Io::Io() {
  base = event_base_new();
  DCHECK_NOTNULL(base);
}

Io::~Io() {
  event_base_free(base);
}

void Io::sleep(int fd, Event event) {
  Executor::Thread thread;

  short eventlib_event;  // NOLINT

  switch (event) {
    case Event::Read:
      eventlib_event = EV_READ;
      break;
    case Event::Write:
      eventlib_event = EV_WRITE;
      break;
    default:
      DCHECK(false) << "Bad event!";
  }

  auto ev = event_new(base, fd, eventlib_event, event_cb, &thread);
  event_add(ev, nullptr);

  Executor::current()->sleep([&](auto thread_) {
    thread = std::move(thread_);
  });

  event_free(ev);
}

void Io::add(Executor *executor) {
  DCHECK_NOTNULL(executor);

  executor->add([&]() {
    // Execute the event loop as long as there is at least one other thread
    // that is or might perform IO.
    while (Executor::current()->alive() > 1) {
      event_base_loop(base, EVLOOP_NONBLOCK);
      Executor::current()->yield();
    }
  });
}

}

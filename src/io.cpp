#include <glog/logging.h>

#include <uthread/io.hpp>

namespace uthread {

thread_local Io *Io::this_io_ = nullptr;

struct IoSleeper {
  Executor::Thread thread;

  Io::Event event;
};

static void event_cb(evutil_socket_t, short event, void *arg) {
  IoSleeper *sleeper = reinterpret_cast<IoSleeper*>(arg);

  if ((event & (EV_READ | EV_WRITE)) == (EV_READ | EV_WRITE)) {
    sleeper->event = Io::Event::ReadWrite;
  } else if (event & EV_READ) {
    sleeper->event = Io::Event::Read;
  } else if (event & EV_WRITE) {
    sleeper->event = Io::Event::Write;
  }

  Executor::current()->ready(std::move(sleeper->thread));
}

Io::Event Io::sleep_on_fd(int fd, Event event) {
  short eventlib_event = 0;

  switch (event) {
    case Event::Read:
      eventlib_event = EV_READ;
      break;
    case Event::Write:
      eventlib_event = EV_WRITE;
      break;
    case Event::ReadWrite:
      eventlib_event = EV_READ | EV_WRITE;
      break;
    default:
      DCHECK(false) << "Bad event!";
  }

  return sleep(fd, eventlib_event, nullptr);
}

void Io::add(Executor *executor) {
  DCHECK_NOTNULL(executor);

  executor->add([&]() {
    auto executor = Executor::current();
    this_io_ = this;

    // Are there any other threads which might perform IO?
    while (executor->alive() > 1) {
      // Are there any other runnable threads we should avoid blocking?
      auto flags = executor->ready() == 0
        ? EVLOOP_ONCE
        : EVLOOP_NONBLOCK;
      auto code = event_base_loop(base_.raw(), flags);
      DCHECK_NE(code, -1);
      executor->yield();
    }

    this_io_ = nullptr;
  });
}

Io* Io::current() {
  DCHECK_NOTNULL(this_io_);

  return this_io_;
}

Io::Event Io::sleep(int fd, short eventlib_event, const timeval *timeout) {
  DCHECK_NE(eventlib_event, 0);

  IoSleeper sleeper;
  char buf[128];
  DCHECK_GE(sizeof(buf), event_get_struct_event_size());
  event *ev = reinterpret_cast<event *>(buf);
  event_assign(ev, base_.raw(), fd, eventlib_event, event_cb, &sleeper);
  event_add(ev, timeout);

  Executor::current()->sleep([&](auto thread_) {
    sleeper.thread = std::move(thread_);
  });

  return sleeper.event;
}

}

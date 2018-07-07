#include <glog/logging.h>

#include <uthread/io.hpp>

namespace uthread {

thread_local Io *Io::this_io_ = nullptr;

struct IoSleeper {
  Executor::Thread thread;

  Io::Event event;
};

static void event_cb(evutil_socket_t, short event, void *arg) {
  IoSleeper *sleeper = reinterpret_cast<IoSleeper *>(arg);

  if ((event & (EV_READ | EV_WRITE)) == (EV_READ | EV_WRITE)) {
    sleeper->event = Io::Event::ReadWrite;
  } else if (event & EV_READ) {
    sleeper->event = Io::Event::Read;
  } else if (event & EV_WRITE) {
    sleeper->event = Io::Event::Write;
  }

  Executor::current()->ready(std::move(sleeper->thread));
}

Io::Io() {
  auto config = event_config_new();
  CHECK_NOTNULL(config);

  CHECK_EQ(event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK), 0);
  CHECK_EQ(event_config_set_flag(config, EVENT_BASE_FLAG_PRECISE_TIMER), 0);

  base = event_base_new_with_config(config);
  CHECK_NOTNULL(base);

  event_config_free(config);
}

Io::~Io() {
  event_base_free(base);
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
    this_io_ = this;

    // Execute the event loop as long as there is at least one other thread
    // that is or might perform IO.
    while (Executor::current()->alive() > 1) {
      // Don't block! We might have other threads to context switch into if no
      // events have triggered. EVLOOP_ONCE was considered for the special case
      // where there are no ready threads but benchmarks showed higher timer
      // latencies and little improvement in CPU time compared to using a busy
      // loop for > 4k sleeping threads. EVLOOP_ONCE MAY be worth considering
      // for cases with predominantly file IO and a smaller number of threads.
      // Another possible optimization is to perform IO multiplexing on a
      // another kernel thread and avoid restarting the loop continuously. This
      // however would required synchronization and might cancel out potential
      // gains. tdlr; these ideas need to be benchmarked :)
      auto code = event_base_loop(base, EVLOOP_NONBLOCK);
      DCHECK_NE(code, -1);
      Executor::current()->yield();
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
  event_assign(ev, base, fd, eventlib_event, event_cb, &sleeper);
  event_add(ev, timeout);

  Executor::current()->sleep([&](auto thread_) {
    sleeper.thread = std::move(thread_);
  });

  return sleeper.event;
}

}

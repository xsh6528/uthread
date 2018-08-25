#include <benchmark/benchmark.h>

#include <uthread/uthread.hpp>

namespace uthread {

struct Arg {
  event *ev;
  int add;
};
constexpr auto kMaxEvs = 1 << 15;

constexpr struct timeval kOneMs = {0, 1000};

Arg args[kMaxEvs] = {};

static void cb(evutil_socket_t, short, void *argraw) {
  auto arg = reinterpret_cast<Arg*>(argraw);
  arg->add--;
  if (arg->add <= 0) return;
  event_add(arg->ev, &kOneMs);
}

/**
 * This benchmark measures both the wall clock and CPU time libevent uses when
 * faced with an increasing heard of timers. We create N 1ms timeout events and
 * activate each one 1000 times, expecting a wall clock time of ~1s for the
 * entire benchmark if there was no overhead.
 */
static void bench_libevent_timer_heard(benchmark::State &state) {
  LibeventBase eb;

  for (Arg *arg = args; arg < args + kMaxEvs; arg++) {
    arg->ev = event_new(eb.raw(), -1, 0, cb, arg);
    CHECK_NOTNULL(arg->ev);
  }

  for (auto _ : state) {
    auto sleepers = state.range(0);
    for (Arg *arg = args; arg < args + sleepers; arg++) {
      arg->add = 1000;
      event_add(arg->ev, &kOneMs);
    }

    event_base_dispatch(eb.raw());
  }

  for (Arg *arg = args; arg < args + kMaxEvs; arg++)
    event_free(arg->ev);
}

BENCHMARK(bench_libevent_timer_heard)
  ->Range(1, 1 << 15)
  ->Unit(benchmark::kMillisecond)
  ->UseRealTime();

}

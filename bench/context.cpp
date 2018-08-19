#include <benchmark/benchmark.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

namespace uthread { namespace context {

static Context context_g;

static void f() {
  context_set(&context_g);
}

static void g() {
  if (context_get(&context_g) == Snapshot::SNAPSHOT) {
    f();
  }
}

/**
 * Benchmarks a context snapshot and switch, using the uthread context_get and
 * context_set API.
 */
static void bench_context_uthread(benchmark::State &state) {
  for (auto _ : state) {
    g();
  }
}

BENCHMARK(bench_context_uthread);

#ifdef __linux__

#include <ucontext.h>

static ucontext_t context_i;

static void h() {
  CHECK_EQ(setcontext(&context_i), 0);
}

static void i() {
  volatile int go_to_h = 1;
  CHECK_EQ(getcontext(&context_i), 0);
  if (go_to_h) {
    go_to_h = 0;
    h();
  }
}

/**
 * Benchmarks a context snapshot and switch, using the Linux getcontext() and
 * setcontext() API.
 */
static void bench_context_linux(benchmark::State &state) {
  for (auto _ : state) {
    i();
  }
}

BENCHMARK(bench_context_linux);

#endif

}}

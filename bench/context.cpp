#include <benchmark/benchmark.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

namespace uthread { namespace context {

/**
 * This benchmarks estimates the cost of a context snapshot and switch, using
 * the uthread context API.
 */
static void bench_context_uthread(benchmark::State &state) {
  for (auto _ : state) {
    Context context;
    if (context_get(&context) == Snapshot::SNAPSHOT) {
      context_set(&context);
    }
  }
}

BENCHMARK(bench_context_uthread);

#ifdef __linux__

#include <ucontext.h>

/**
 * This benchmarks estimates the cost of a context snapshot and switch, using
 * the Linux context API.
 */
static void bench_context_linux(benchmark::State &state) {
  for (auto _ : state) {
    ucontext_t context;
    volatile bool skip = false;
    CHECK_EQ(getcontext(&context), 0);
    if (!skip) {
      skip = true;
      CHECK_EQ(setcontext(&context), 0);
    }
  }
}

BENCHMARK(bench_context_linux);

#endif

}}

#include <benchmark/benchmark.h>

#include <uthread/uthread.hpp>

namespace uthread {

static void sleep_user_threads_args(benchmark::internal::Benchmark* bench) {
  for (int threads = 1; threads <= 32768; threads *= 8) {
    for (int sleeps = 1; sleeps <= 512; sleeps *= 8) {
      bench->Args({threads, sleeps});
    }
  }
}

/**
 * This benchmark estimates the cost of sleeping from many threads. There are
 * N threads sleeping M times for 1 ms. In an ideal case, this benchmark would
 * take M ms, but of course there is scheduling and context switching overhead
 * with more threads and sleep calls. That's what we are measuring here.
 */
static void sleep_user_threads(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    Executor exe;

    Io io;
    io.add(&exe);

    for (int threads = 0; threads < state.range(0); threads++) {
      exe.add([&]() {
        for (int sleeps = 0; sleeps < state.range(1); sleeps++) {
          io.sleep_for(std::chrono::milliseconds(1));
        }
      });
    }

    state.ResumeTiming();
    exe.run();
  }
}

BENCHMARK(sleep_user_threads)
  ->Apply(sleep_user_threads_args)
  ->UseRealTime()
  ->Unit(benchmark::kMillisecond);

}

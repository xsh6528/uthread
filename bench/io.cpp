#include <benchmark/benchmark.h>

#include <uthread/uthread.hpp>

namespace uthread {

/**
 * This benchmark estimates the cost of sleeping from many threads. There are
 * N threads sleeping M times for 1 ms. In an ideal case, this benchmark would
 * take M ms, but of course there is scheduling and context switching overhead
 * with more threads and sleep calls. That's what we are measuring here.
 */
template<Io::Timer kTimer>
static void bench_io_sleep(benchmark::State &state) {
  for (auto _ : state) {
    state.PauseTiming();
    Executor exe;
    Io io;
    io.add(&exe);

    for (int threads = 0; threads < state.range(0); threads++) {
      exe.add([&]() {
        for (int sleeps = 0; sleeps < state.range(1); sleeps++) {
          io.sleep_for(std::chrono::milliseconds(1), kTimer);
        }
      });
    }

    state.ResumeTiming();
    exe.run();
  }
}

BENCHMARK_TEMPLATE(bench_io_sleep, Io::Timer::Libevent)
  ->Ranges({{1, 1<<15}, {1, 512}})
  ->Unit(benchmark::kMillisecond)
  ->UseRealTime();

BENCHMARK_TEMPLATE(bench_io_sleep, Io::Timer::CpuLoop)
  ->Ranges({{1, 1<<15}, {1, 512}})
  ->Unit(benchmark::kMillisecond)
  ->UseRealTime();

}

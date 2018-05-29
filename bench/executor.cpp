#include <benchmark/benchmark.h>

#include <uthread/uthread.hpp>

namespace uthread {

/**
 * This benchmark estimates the cost of two yields. The first yield context
 * switches from thread one to thread two and the second yield switches in the
 * opposite direction. There is some overhead in the estimate so the true cost
 * of a yield is "a bit less" than half of the time this benchmark estimates.
 */
static void bench_yield_twice(benchmark::State& state) {
  Executor exe;

  exe.add([&]() {
    for (auto _ : state) {
      Executor::current()->yield();
    }
  });

  exe.add([&]() {
    while (Executor::current()->alive() == 2) {
      Executor::current()->yield();
    }
  });

  exe.run();
}

BENCHMARK(bench_yield_twice);

}

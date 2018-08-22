#include <benchmark/benchmark.h>

#include <uthread/uthread.hpp>

namespace uthread {

/**
 * This benchmark estimates the cost of yielding from one thread to another and
 * back.
 */
static void bench_executor_yield_pong(benchmark::State &state) {
  bool benching = true;
  Executor exe;

  exe.add([&]() {
    for (auto _ : state) {
      Executor::get()->yield();
    }
    benching = false;
  });

  exe.add([&]() {
    while (benching) {
      Executor::get()->yield();
    }
  });

  exe.run();
}

/**
 * This benchmark estimates the cost of sleeping a thread and readying it up
 * from another.
 */
static void bench_executor_sleep_and_ready(benchmark::State &state) {
  bool benching = true;
  Executor::Thread thread;
  Executor exe;

  exe.add([&]() {
    for (auto _ : state) {
      Executor::get()->sleep([&](auto thread_) {
        thread = std::move(thread_);
      });
    }
    benching = false;
  });

  exe.add([&]() {
    auto executor = Executor::get();
    if (!thread)
      executor->yield();
    while (benching) {
      executor->ready(std::move(thread));
      executor->yield();
    }
  });

  exe.run();
}

BENCHMARK(bench_executor_yield_pong);
BENCHMARK(bench_executor_sleep_and_ready);

}

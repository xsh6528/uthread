#include <ucontext.h>

#include <benchmark/benchmark.h>
#include <glog/logging.h>

#include <uthread/context.hpp>

namespace uthread { namespace context {

static Context context_g;

static ucontext_t context_i;

static void f() {
  run(&context_g);
}

static void g() {
  if (snapshot(&context_g) == Snapshot::SNAPSHOT) {
    f();
  }
}

/**
 * Benchmarks a snapshot() + run() (a single context switch) using our own
 * context API.
 */
static void bench_uthread_switch(benchmark::State& state) {
  for (auto _ : state) {
    g();
  }
}

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
 * Benchmarks a getcontext() + setcontext() (a single context switch) using
 * Linux's buildin ucontext API.
 */
static void bench_sys_switch(benchmark::State& state) {
  for (auto _ : state) {
    i();
  }
}

BENCHMARK(bench_uthread_switch);
BENCHMARK(bench_sys_switch);

}}

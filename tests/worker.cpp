#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

static constexpr int DESTROY = 0;
static constexpr int FOO = 1;
static int ops[3] = {0, 0, 0};

struct Thingy {
  ~Thingy() {
    ops[DESTROY]++;
  }

  void foo() {
    ops[FOO]++;
  }
};

TEST(WorkerTest, RunSingleThread) {
  int x = 0;
  Worker worker;

  auto f = Thread::create([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  worker.schedule(std::move(f));
  worker.loop();

  ASSERT_EQ(x, 1);
}

TEST(WorkerTest, SwitchBetweenThreads) {
  int x = 0;
  Worker worker;

  auto f = Thread::create([&]() {
    ASSERT_EQ(x, 0);
    worker.yield();
    ASSERT_EQ(x, 1);
    worker.yield();
    ASSERT_EQ(x, 2);
  });

  auto g = Thread::create([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    worker.yield();
    ASSERT_EQ(x, 1);
    x = 2;
    worker.yield();
    ASSERT_EQ(x, 2);
  });

  worker.schedule(std::move(f));
  worker.schedule(std::move(g));
  worker.loop();

  ASSERT_EQ(x, 2);
}

TEST(WorkerTest, SwitchToNextOnFinish) {
  int x = 0;
  Worker worker;

  auto f = Thread::create([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  auto g = Thread::create([&]() {
    ASSERT_EQ(x, 1);
    x = 2;
  });

  worker.schedule(std::move(f));
  worker.schedule(std::move(g));
  worker.loop();

  ASSERT_EQ(x, 2);
}

TEST(WorkerTest, ThreadLifetime) {
  Thingy x;
  Worker worker;

  ASSERT_EQ(ops[DESTROY], 0);
  ASSERT_EQ(ops[FOO], 0);

  auto f = Thread::create([=]() mutable {
    x.foo();
  });

  auto g = Thread::create([=]() mutable {
    x.foo();
  });

  // https://lonelycoding.com/move-semantic-with-stdfunction-2/
  //
  // tldr; We can't guarantee how many times x has been copied and destroyed
  // in the thread lambda.
  auto destroyed = ops[DESTROY];

  worker.schedule(std::move(f));
  worker.schedule(std::move(g));
  worker.loop();

  // Threads have ran, make sure closure, etc. have been destroyed.
  ASSERT_EQ(ops[DESTROY], destroyed + 2);
  ASSERT_EQ(ops[FOO], 2);
}

}

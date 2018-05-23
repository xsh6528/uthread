#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

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

}

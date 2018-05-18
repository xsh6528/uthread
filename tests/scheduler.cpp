#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(SchedulerTest, RunSingleThread) {
  int x = 0;
  auto scheduler = std::make_unique<RoundRobin>(64 * 1024);

  scheduler->spawn([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  scheduler->run();

  ASSERT_EQ(x, 1);
}


TEST(SchedulerTest, SwitchBetweenThreads) {
  int x = 0;
  auto scheduler = std::make_unique<RoundRobin>(64 * 1024);

  auto f = [&]() {
    ASSERT_EQ(x, 0);
    scheduler->yield();
    ASSERT_EQ(x, 1);
    scheduler->yield();
    ASSERT_EQ(x, 2);
    scheduler->yield();
  };

  auto g = [&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    scheduler->yield();
    ASSERT_EQ(x, 1);
    x = 2;
    scheduler->yield();
    ASSERT_EQ(x, 2);
  };

  scheduler->spawn(f);
  scheduler->spawn(g);
  scheduler->run();

  ASSERT_EQ(x, 2);
}

TEST(SchedulerTest, SwitchToNextOnFinish) {
  int x = 0;
  auto scheduler = std::make_unique<RoundRobin>(64 * 1024);

  auto f = [&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  };

  auto g = [&]() {
    ASSERT_EQ(x, 1);
    x = 2;
  };

  scheduler->spawn(f);
  scheduler->spawn(g);
  scheduler->run();

  ASSERT_EQ(x, 2);
}

}

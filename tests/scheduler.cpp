#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(SchedulerTest, RunSingleThread) {
  int x = 0;
  auto scheduler = std::make_unique<RoundRobin>();

  Thread f([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  scheduler->schedule(std::move(f));
  scheduler->run();

  ASSERT_EQ(x, 1);
}

TEST(SchedulerTest, SwitchBetweenThreads) {
  int x = 0;
  auto scheduler = std::make_unique<RoundRobin>();

  Thread f([&]() {
    ASSERT_EQ(x, 0);
    scheduler->yield();
    ASSERT_EQ(x, 1);
    scheduler->yield();
    ASSERT_EQ(x, 2);
  });

  Thread g([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    scheduler->yield();
    ASSERT_EQ(x, 1);
    x = 2;
    scheduler->yield();
    ASSERT_EQ(x, 2);
  });

  scheduler->schedule(std::move(f));
  scheduler->schedule(std::move(g));
  scheduler->run();

  ASSERT_EQ(x, 2);
}

TEST(SchedulerTest, SwitchToNextOnFinish) {
  int x = 0;
  auto scheduler = std::make_unique<RoundRobin>();

  Thread f([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  Thread g([&]() {
    ASSERT_EQ(x, 1);
    x = 2;
  });

  scheduler->schedule(std::move(f));
  scheduler->schedule(std::move(g));
  scheduler->run();

  ASSERT_EQ(x, 2);
}

}

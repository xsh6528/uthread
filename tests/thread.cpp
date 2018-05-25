#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(ThreadTest, RunSingleThread) {
  int x = 0;

  Thread::run([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  ASSERT_EQ(x, 1);
}

TEST(ThreadTest, SpawnOneThread) {
  int x = 0;

  Thread::run([&]() {
    Thread::spawn([&]() {
      ASSERT_EQ(x, 1);
      x = 2;
    });
    ASSERT_EQ(x, 0);
    x = 1;
  });

  ASSERT_EQ(x, 2);
}

TEST(ThreadTest, SpawnTwoThreadsAndYield) {
  int x = 0;

  auto f = [&]() {
    ASSERT_EQ(x, 0);
    Thread::yield();
    ASSERT_EQ(x, 1);
    Thread::yield();
    ASSERT_EQ(x, 2);
  };

  auto g = [&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    Thread::yield();
    ASSERT_EQ(x, 1);
    x = 2;
    Thread::yield();
    ASSERT_EQ(x, 2);
  };

  Thread::run([&]() {
    Thread::spawn(std::move(f));
    Thread::spawn(std::move(g));
  });

  ASSERT_EQ(x, 2);
}

TEST(ThreadTest, SleepThreadThenReady) {
  int x = 0;

  Thread::run([&]() {
    Thread::Queue queue;

    Thread::spawn([&]() {
      ASSERT_EQ(x, 0);
      x = 1;
      Thread::sleep(&queue);

      ASSERT_EQ(x, 2);
      x = 3;
    });

    ASSERT_EQ(x, 0);
    Thread::yield();

    ASSERT_EQ(x, 1);
    x = 2;
    Thread::ready(std::move(queue.front()));
    Thread::yield();

    ASSERT_EQ(x, 3);
  });

  ASSERT_EQ(x, 3);
}

TEST(ThreadTest, DeathIfSleepingWithNoReadyThreads) {
  Thread::run([]() {
    Thread::Queue queue;
    ASSERT_DEATH(Thread::sleep(&queue), "deadlock");
  });
}

TEST(ThreadTest, JoinOnRunningThread) {
  int x = 0;

  Thread::run([&]() {
    auto f = Thread::spawn([&]() {
      ASSERT_EQ(x, 0);
      x = 1;
      for (int i = 0; i < 100; i++) {
        Thread::yield();
        ASSERT_EQ(x, 1);
      }
      x = 2;
    });

    auto g = Thread::spawn([&]() {
      f.join();
      ASSERT_EQ(x, 2);
      x = 3;
    });

    f.join();
    g.join();

    ASSERT_EQ(x, 3);
  });

  ASSERT_EQ(x, 3);
}

TEST(ThreadTest, JoinOnFinishedThread) {
  int x = 0;

  Thread::run([&]() {
    auto f = Thread::spawn([&]() {
      ASSERT_EQ(x, 0);
      x = 1;
    });

    Thread::yield();
    ASSERT_EQ(x, 1);

    auto g = Thread::spawn([&]() {
      f.join();
      ASSERT_EQ(x, 1);
      x = 2;
    });

    f.join();
    g.join();

    ASSERT_EQ(x, 2);
  });

  ASSERT_EQ(x, 2);
}

TEST(ThreadTest, ThreadDestroyedOnFinish) {
  auto x = std::make_shared<int>();

  Thread::run([=]() mutable {
    Thread::spawn([=]() mutable {
      ASSERT_EQ(x.use_count(), 3);
      ASSERT_EQ(*x, 1);
      *x = 2;
    });

    Thread::spawn([=]() mutable {
      ASSERT_EQ(x.use_count(), 2);
      ASSERT_EQ(*x, 2);
      *x = 3;
    });

    ASSERT_EQ(x.use_count(), 4);
    ASSERT_EQ(*x, 0);
    *x = 1;
  });

  ASSERT_EQ(x.use_count(), 1);
  ASSERT_EQ(*x, 3);
}

}

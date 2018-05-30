#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

TEST(ExecutorTest, RunSingleThread) {
  int x = 0;
  Executor exe;

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  exe.run();

  ASSERT_EQ(x, 1);
}

TEST(ExecutorTest, SpawnTwoThreadsAndYield) {
  int x = 0;
  Executor exe;

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    Executor::current()->yield();
    ASSERT_EQ(x, 1);
    Executor::current()->yield();
    ASSERT_EQ(x, 2);
  });

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    Executor::current()->yield();
    ASSERT_EQ(x, 1);
    x = 2;
    Executor::current()->yield();
    ASSERT_EQ(x, 2);
  });

  exe.run();

  ASSERT_EQ(x, 2);
}

TEST(ExecutorTest, SleepThreadThenReady) {
  int x = 0;
  Executor exe;
  Executor::Thread thread;

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    Executor::current()->yield();
    ASSERT_EQ(x, 1);
    x = 2;
    Executor::current()->add(std::move(thread));
    Executor::current()->yield();
    ASSERT_EQ(x, 3);
  });

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    Executor::current()->sleep([&](auto thread_) {
      thread = std::move(thread_);
    });
    ASSERT_EQ(x, 2);
    x = 3;
  });

  exe.run();

  ASSERT_EQ(x, 3);
}

TEST(ExecutorTest, DeathIfSleepingWithNoReadyThreads) {
  Executor exe;
  Executor::Thread thread;

  exe.add([&]() {
    Executor::current()->sleep([&](auto _) {});
  });

  ASSERT_DEATH(exe.run(), "deadlock");
}

TEST(ExecutorTest, JoinOnRunningThread) {
  int x = 0;
  Executor exe;
  Executor::Thread thread;

  auto f = exe.add([&]() {
    ASSERT_EQ(x, 0);
    while (x < 100) {
      Executor::current()->yield();
    }
  });

  auto g = exe.add([&]() {
    ASSERT_EQ(x, 0);
    while (x < 100) {
      x++;
      Executor::current()->yield();
    }
  });

  auto h = exe.add([&]() {
    f.join();
    g.join();

    ASSERT_EQ(x, 100);
  });

  exe.run();

  ASSERT_EQ(x, 100);
}

TEST(ExecutorTest, JoinOnFinishedThread) {
  int x = 0;
  Executor exe;
  Executor::Thread thread;

  auto f = exe.add([&]() {
    ASSERT_EQ(x, 0);
    while (x < 100) {
      Executor::current()->yield();
    }
  });

  auto g = exe.add([&]() {
    ASSERT_EQ(x, 0);
    while (x < 100) {
      x++;
      Executor::current()->yield();
    }

    f.join();

    ASSERT_EQ(x, 100);
  });

  exe.run();

  ASSERT_EQ(x, 100);
}

TEST(ExecutorTest, ThreadDestroyedOnFinish) {
  auto x = std::make_shared<int>();
  Executor exe;

  auto f = exe.add([=]() {
    ASSERT_EQ(x.use_count(), 4);
    ASSERT_EQ(*x, 0);
    *x = 1;
  });

  auto g = exe.add([=]() {
    ASSERT_EQ(x.use_count(), 3);
    ASSERT_EQ(*x, 1);
    *x = 2;
  });

  auto h = exe.add([=]() {
    ASSERT_EQ(x.use_count(), 2);
    ASSERT_EQ(*x, 2);
    *x = 3;
  });

  exe.run();

  ASSERT_EQ(x.use_count(), 1);
  ASSERT_EQ(*x, 3);
}

TEST(ExecutorTest, DeathFromExceptionCleansUpOtherThreads) {
  Executor exe;

  exe.add([=]() {
    throw std::runtime_error("This is an exception!");
  });

  ASSERT_DEATH(exe.run(), "An exception has occurred: This is an exception!");
}

}

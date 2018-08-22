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
    Executor::get()->yield();
    ASSERT_EQ(x, 1);
    Executor::get()->yield();
    ASSERT_EQ(x, 2);
  });

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    Executor::get()->yield();
    ASSERT_EQ(x, 1);
    x = 2;
    Executor::get()->yield();
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
    Executor::get()->yield();
    ASSERT_EQ(x, 1);
    x = 2;
    Executor::get()->ready(std::move(thread));
    Executor::get()->yield();
    ASSERT_EQ(x, 3);
  });

  exe.add([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
    Executor::get()->sleep([&](auto thread_) {
      thread = std::move(thread_);
    });
    ASSERT_EQ(x, 2);
    x = 3;
  });

  exe.run();

  ASSERT_EQ(x, 3);
}

TEST(ExecutorTest, JoinOnRunningThread) {
  int x = 0;
  Executor exe;
  Executor::Thread thread;

  auto f = exe.add([&]() {
    ASSERT_EQ(x, 0);
    Executor::get()->yield();
    ASSERT_EQ(x, 0);
    x = 1;
  });

  auto g = exe.add([&]() {
    ASSERT_EQ(x, 0);
    f.join();
    ASSERT_EQ(x, 1);
    x = 2;
  });

  exe.run();

  ASSERT_EQ(x, 2);
}

TEST(ExecutorTest, JoinOnFinishedThread) {
  int x = 0;
  Executor exe;
  Executor::Thread thread;

  auto f = exe.add([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  auto g = exe.add([&]() {
    while (x != 1) {
      Executor::get()->yield();
    }
    f.join();
    x = 2;
  });

  exe.run();

  ASSERT_EQ(x, 2);
}

TEST(ExecutorTest, ThreadStackedCleanedOnFinish) {
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

TEST(ExecutorTest, DeathIfSleepingWithNoReadyThreads) {
  Executor exe;

  exe.add([]() {
    Executor::get()->sleep([](auto _) {});
  });

  ASSERT_DEATH(exe.run(), "Deadlock has occurred: No ready threads!");
}

TEST(ExecutorTest, DeathIfExceptionInThread) {
  Executor exe;

  exe.add([]() {
    throw std::runtime_error("This is an exception!");
  });

  ASSERT_DEATH(exe.run(), "An exception has occurred: This is an exception!");
}

TEST(ExecutorTest, DeathIfZombieThread) {
  Executor exe;

  auto f = []() {
    Executor::Thread thread;
    Executor::get()->sleep([&](auto thread_) {
      /** Thread is placed on it's own stack and never resumed! */
      thread = std::move(thread_);
    });
  };

  auto g = [&]() {
    auto exe = Executor::get();
    exe->add(f);
    exe->yield();
  };

  exe.add(g);
  ASSERT_DEATH(exe.run(), "Zombie threads: This poses a resource leak!");
}

}

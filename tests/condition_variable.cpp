#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

TEST(ConditionVariableTest, WakeOne) {
  int x = 0;
  Mutex mutex;
  ConditionVariable cv;
  Executor exe;

  for (int i = 0; i < 100; i++) {
    exe.add([&]() {
      Lock guard(&mutex);
      ASSERT_LT(x, 100);
      x++;
      cv.sleep(&guard);
      x++;
    });
  }

  exe.add([&]() {
    // Wait for all threads to sleep..
    while (x != 100) {
      Executor::get()->yield();
    }

    // Now let's make sure the threads stay asleep...
    for (int i = 0; i < 100; i++) {
      ASSERT_EQ(x, 100);
      Executor::get()->yield();
    }

    // Wake one at a time...
    for (int i = 0; i < 100; i++) {
      cv.wake_one();
      Executor::get()->yield();
      ASSERT_EQ(x, 101 + i);
    }
  });

  exe.run();

  ASSERT_EQ(x, 200);
}

TEST(ConditionVariableTest, WakeAll) {
  int x = 0;
  Mutex mutex;
  ConditionVariable cv;
  Executor exe;

  for (int i = 0; i < 100; i++) {
    exe.add([&]() {
      Lock guard(&mutex);
      ASSERT_LT(x, 100);
      x++;
      cv.sleep(&guard);
      x++;
    });
  }

  exe.add([&]() {
    // Wait for all threads to sleep..
    while (x != 100) {
      Executor::get()->yield();
    }

    // Now let's make sure the threads stay asleep...
    for (int i = 0; i < 100; i++) {
      ASSERT_EQ(x, 100);
      Executor::get()->yield();
    }

    // Wake as a group...
    cv.wake_all();
    while (x != 200) {
      Executor::get()->yield();
    }
  });

  exe.run();

  ASSERT_EQ(x, 200);
}

}

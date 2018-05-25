#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(ConditionVariableTest, WakeOne) {
  int x = 0;
  Mutex mutex;
  ConditionVariable cv;

  Thread::run([&]() {
    for (int i = 0; i < 100; i++) {
      Thread::spawn([&]() {
        Lock guard(&mutex);
        ASSERT_LT(x, 100);
        x++;
        cv.sleep(&guard);
        x++;
      });
    }

    // Wait for all threads to sleep.
    while (x != 100) {
      Thread::yield();
    }

    // Let's make sure the threads stay asleep.
    for (int i = 0; i < 100; i++) {
      ASSERT_EQ(x, 100);
      Thread::yield();
    }

    // Wake one at a time.
    for (int i = 0; i < 100; i++) {
      cv.wake_one();
      Thread::yield();
      ASSERT_EQ(x, 101 + i);
    }
  });

  ASSERT_EQ(x, 200);
}

TEST(ConditionVariableTest, WakeAll) {
  int x = 0;
  Mutex mutex;
  ConditionVariable cv;

  Thread::run([&]() {
    for (int i = 0; i < 100; i++) {
      Thread::spawn([&]() {
        Lock guard(&mutex);
        ASSERT_LT(x, 100);
        x++;
        cv.sleep(&guard);
        x++;
      });
    }

    // Wait for all threads to sleep.
    while (x != 100) {
      Thread::yield();
    }

    // Let's make sure the threads stay asleep.
    for (int i = 0; i < 100; i++) {
      ASSERT_EQ(x, 100);
      Thread::yield();
    }

    // Wake threads as a group.
    cv.wake_all();
    while (x != 200) {
      Thread::yield();
    }
  });

  ASSERT_EQ(x, 200);
}

}

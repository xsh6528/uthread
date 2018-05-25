#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(ConditionVariableTest, WakeOne) {
  int x = 0;
  Mutex mutex;
  ConditionVariable cv;

  Thread::run([&]() {
    Thread::spawn([&]() {
      Lock guard(&mutex);
      ASSERT_EQ(x, 0);
      x = 1;
      cv.sleep(&guard);
      ASSERT_EQ(x, 2);
      x = 3;
    });

    Thread::spawn([&]() {
      ASSERT_EQ(x, 1);
      x = 2;
      cv.wake_one();
      ASSERT_EQ(x, 2);
    });
  });

  ASSERT_EQ(x, 3);
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
        ASSERT_GT(x, 100);
        x++;
      });
    }

    Thread::spawn([&]() {
      ASSERT_EQ(x, 100);
      x = 101;
      cv.wake_all();
      ASSERT_EQ(x, 101);
    });
  });

  ASSERT_EQ(x, 201);
}

}

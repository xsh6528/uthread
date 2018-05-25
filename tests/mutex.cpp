#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(MutexTest, AcquireWithoutContension) {
  int x = 0;
  Mutex mutex;

  Thread::run([&]() {
    Lock guard(&mutex);
    x = 1;
  });

  ASSERT_EQ(x, 1);
}

TEST(MutexTest, AcquireWithContension) {
  int x = 0;
  Mutex mutex;

  Thread::run([&]() {
    for (int i = 0; i < 100; i++) {
      Thread::spawn([&]() {
        Lock guard(&mutex);

        x++;
        auto copy = x;

        for (int j = 0; j < 100; j++) {
          Thread::yield();
          ASSERT_EQ(x, copy);
        }
      });
    }
  });

  ASSERT_EQ(x, 100);
}

}

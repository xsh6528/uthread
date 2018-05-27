#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(MutexTest, AcquireWithoutContension) {
  int x = 0;
  Mutex mutex;
  Executor exe;

  exe.add([&]() {
    Lock guard(&mutex);
    x = 1;
  });

  exe.run();

  ASSERT_EQ(x, 1);
}

TEST(MutexTest, AcquireWithContension) {
  int x = 0;
  Mutex mutex;
  Executor exe;

  for (int i = 0; i < 100; i++) {
    exe.add([&]() {
      Lock guard(&mutex);

      x++;
      auto copy = x;

      // Let's make sure the lock keeps other threads away...
      for (int j = 0; j < 100; j++) {
        Executor::current()->yield();
        ASSERT_EQ(x, copy);
      }
    });
  }

  exe.run();

  ASSERT_EQ(x, 100);
}

}

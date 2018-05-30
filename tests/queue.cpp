#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

static constexpr auto k100Ms = std::chrono::milliseconds(100);

TEST(QueueTest, UnboundedPushAndPopFromSingleThread) {
  Executor exe;

  MpmcQueue<int> q;

  exe.add([&]() {
    for (int i = 0; i < 100; i++) {
      q.push(i);
    }

    for (int i = 0; i < 100; i++) {
      int x;
      q.pop(x);
      ASSERT_EQ(x, i);
    }
  });

  exe.run();
}

TEST(QueueTest, PopsWakeManySleepers) {
  Executor exe;

  Io io;
  io.add(&exe);

  MpmcQueue<int> q(100);

  exe.add([&]() {
    io.sleep_for(k100Ms);
    for (int i = 0; i < 100; i++) {
      ASSERT_TAKES_APPROX_MS(q.push(-1), 0);
    }
  });

  for (int i = 0; i < 100; i++) {
    exe.add([&]() {
      int x;
      ASSERT_TAKES_APPROX_MS(q.pop(x), 100);
      ASSERT_EQ(x, -1);
    });
  }

  ASSERT_TAKES_APPROX_MS(exe.run(), 100);
}

TEST(QueueTest, PushWakeManySleepers) {
  Executor exe;

  Io io;
  io.add(&exe);

  MpmcQueue<int> q(1);

  exe.add([&]() {
    ASSERT_TAKES_APPROX_MS(q.push(-1), 0);
    io.sleep_for(k100Ms);
    for (int i = 0; i < 100; i++) {
      int x;
      ASSERT_TAKES_APPROX_MS(q.pop(x), 0);
      ASSERT_EQ(x, -1);
    }
  });

  for (int i = 0; i < 100; i++) {
    exe.add([&]() {
      ASSERT_TAKES_APPROX_MS(q.push(-1), 100);
    });
  }

  ASSERT_TAKES_APPROX_MS(exe.run(), 100);
}

}

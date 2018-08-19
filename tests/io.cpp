#include <cstring>

#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

class IoTest : public ::testing::TestWithParam<Io::Timer> {};

TEST(IoTest, IoShutsDownIfNoReadyThreads) {
  Executor exe;
  Io io;
  io.add(&exe);

  exe.run();
}

TEST(IoTest, TcpEcho) {
  static constexpr auto kMessageLen = 1024;

  Executor exe;
  Io io;
  io.add(&exe);

  exe.add([&]() {
    TcpListener listener;
    ASSERT_EQ(listener.bind("127.0.0.1", 32000), 0);

    TcpStream stream;
    ASSERT_EQ(listener.accept(stream), 0);

    char buf[kMessageLen];
    int p = 0;
    while (p < kMessageLen) {
      auto r = stream.recv(buf + p, kMessageLen - p);
      ASSERT_GT(r, 0);
      p += r;
    }

    ASSERT_EQ(stream.send(buf, kMessageLen), kMessageLen);
  });

  exe.add([&]() {
    TcpStream stream;
    ASSERT_EQ(stream.connect("127.0.0.1", 32000), 0);

    char send_buf[kMessageLen];
    for (int p = 0; p < kMessageLen; p++)
      send_buf[p] = p;

    ASSERT_EQ(stream.send(send_buf, kMessageLen), kMessageLen);

    char recv_buf[kMessageLen];
    int p = 0;
    while (p < kMessageLen) {
      auto r = stream.recv(recv_buf + p, kMessageLen - p);
      ASSERT_GT(r, 0);
      p += r;
    }

    ASSERT_EQ(std::memcmp(send_buf, recv_buf, sizeof(send_buf)), 0);
  });

  exe.run();
}

TEST_P(IoTest, SleepsForApproximateTime) {
  static constexpr auto kSleepTime =  std::chrono::milliseconds(100);

  Executor exe;
  Io io;
  io.add(&exe);

  exe.add([&]() {
    ASSERT_TAKES_APPROX_MS(io.sleep_for(kSleepTime, GetParam()), 100);
  });

  ASSERT_TAKES_APPROX_MS(exe.run(), 100);
}

INSTANTIATE_TEST_CASE_P(
  SleepTimers,
  IoTest,
  ::testing::Values(Io::Timer::Libevent, Io::Timer::CpuLoop));

}

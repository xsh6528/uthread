#include <cstring>

#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

TEST(SocketTest, LocalhostEcho) {
  static constexpr auto kMessageLen = 1024;

  Executor exe;
  Io io;
  io.add(&exe);

  exe.add([&]() {
    TcpListener listener;
    TcpBindOptions options;
    options.allow_addr_reuse = true;
    ASSERT_EQ(listener.bind("127.0.0.1", 32000, options), 0);

    TcpStream stream;
    ASSERT_EQ(listener.accept(stream), 0);

    char buf[kMessageLen];
    int p = 0;
    while (p < kMessageLen) {
      auto r = stream.recv(buf + p, kMessageLen - p);
      ASSERT_GT(r, 0);
      p += r;
    }

    ASSERT_EQ(stream.send(buf, kMessageLen), 0);
  });

  exe.add([&]() {
    TcpStream stream;
    ASSERT_EQ(stream.connect("127.0.0.1", 32000), 0);

    char send_buf[kMessageLen];
    for (int p = 0; p < kMessageLen; p++)
      send_buf[p] = p;

    ASSERT_EQ(stream.send(send_buf, kMessageLen), 0);

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

TEST(SocketTest, GoogleDnsConnection) {
  Executor exe;
  Io io;
  io.add(&exe);

  exe.add([]() {
    TcpStream stream;
    ASSERT_EQ(stream.connect("8.8.8.8", 53), 0)
      << "This test connects to Google public DNS. Do you have an internet "
      << "connection? Is Google DNS down (unlikely)? ";
  });

  exe.run();
}

}

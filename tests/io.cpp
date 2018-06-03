#include <cstring>
#include <random>

#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

TEST(IoTest, IoShutsDownIfNoReadyThreads) {
  Executor exe;

  Io io;
  io.add(&exe);

  exe.run();
}

TEST(IoTest, UdpEcho) {
  static constexpr auto kUdpMessageSize = 1024;

  Executor exe;

  Io io;
  io.add(&exe);

  std::random_device r;
  std::default_random_engine e(r());
  std::uniform_int_distribution<int> d(1024, 65535);

  auto server_addr = nix::socket_addr("127.0.0.1", d(e));

  // UDP echo server, waits for one packet, responds, and shuts down.
  exe.add([&]() {
    int fd;
    ASSERT_NE(fd = nix::socket(SOCK_DGRAM), -1);
    ASSERT_NE(::bind(fd, (sockaddr *) &server_addr, sizeof(server_addr)), -1);

    io.sleep_on_fd(fd, Io::Event::Read);

    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    char recv_buf[kUdpMessageSize + 1];
    auto recv_len = ::recvfrom(fd, recv_buf, sizeof(recv_buf), 0,
      (sockaddr *) &src_addr, &src_addr_len);

    ASSERT_NE(recv_len, -1);

    io.sleep_on_fd(fd, Io::Event::Write);

    auto send_len = ::sendto(fd, recv_buf, recv_len, 0,
      (sockaddr *) &src_addr, src_addr_len);

    ASSERT_EQ(send_len, recv_len);

    close(fd);
  });

  // Client that tests the echo server.
  exe.add([&]() {
    int fd;
    ASSERT_NE(fd = nix::socket(SOCK_DGRAM), -1);

    char send_buf[kUdpMessageSize];
    for (size_t i = 0; i < sizeof(send_buf); i++) {
      send_buf[i] = d(e);
    }

    io.sleep_on_fd(fd, Io::Event::Write);

    auto send_len = ::sendto(fd, send_buf, sizeof(send_buf), 0,
      (sockaddr *) &server_addr, sizeof(server_addr));

    ASSERT_EQ(send_len, sizeof(send_buf));

    io.sleep_on_fd(fd, Io::Event::Read);

    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    char recv_buf[sizeof(send_buf) + 1];
    auto recv_len = ::recvfrom(fd, recv_buf, sizeof(recv_buf), 0,
      (sockaddr *) &src_addr, &src_addr_len);

    ASSERT_EQ(recv_len, sizeof(send_buf));
    ASSERT_EQ(std::memcmp(send_buf, recv_buf, sizeof(send_buf)), 0);
    ASSERT_EQ(src_addr.sin_addr.s_addr, server_addr.sin_addr.s_addr);
    ASSERT_EQ(src_addr.sin_port, server_addr.sin_port);

    close(fd);
  });

  exe.run();
}

TEST(IoTest, SleepsForApproximateTime) {
  static constexpr auto kSleepTime =  std::chrono::milliseconds(100);

  Executor exe;

  Io io;
  io.add(&exe);

  exe.add([&]() {
    ASSERT_TAKES_APPROX_MS(io.sleep_for(kSleepTime), 100);
  });

  ASSERT_TAKES_APPROX_MS(exe.run(), 100);
}


}

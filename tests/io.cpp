#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <sys/socket.h>

#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

static constexpr int UDP_MESSAGE_SIZE = 1024;

sockaddr_in addr(int port) {
  sockaddr_in server_addr;
  ::bzero((char *) &server_addr, sizeof(server_addr));  // NOLINT
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
  server_addr.sin_port = htons(port);
  return server_addr;
}

int socket(int type) {
  int fd = ::socket(AF_INET, type, 0);
  CHECK_NE(fd, -1);

  int opt = 1;
  CHECK_NE(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt,
    sizeof(opt)), -1);

  int flags = ::fcntl(fd, F_GETFL);
  CHECK_NE(flags, -1);
  CHECK_NE(::fcntl(fd, F_SETFL, flags | O_NONBLOCK), -1);

  return fd;
}

int server(int type, int port) {
  int fd = socket(type);
  auto server_addr = addr(port);
  CHECK_NE(::bind(fd, (sockaddr *) &server_addr, sizeof(server_addr)), -1);  // NOLINT
  return fd;
}

TEST(IoTest, IoShutsDownIfNoReadyThreads) {
  Executor exe;

  Io io;
  io.add(&exe);

  exe.run();
}

TEST(IoTest, UdpEcho) {
  std::random_device r;
  std::default_random_engine e(r());
  std::uniform_int_distribution<int> d(1024, 65535);

  auto port = d(e);

  Executor exe;

  Io io;
  io.add(&exe);

  // UDP echo server, waits for one packet, responds, and shuts down.
  exe.add([&]() {
    int fd = server(SOCK_DGRAM, port);

    io.sleep(fd, Io::Event::Read);

    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    char recv_buf[UDP_MESSAGE_SIZE + 1];
    auto recv_len = ::recvfrom(fd, recv_buf, sizeof(recv_buf), 0,
      (sockaddr *) &src_addr, &src_addr_len);  // NOLINT

    io.sleep(fd, Io::Event::Write);

    auto send_len = ::sendto(fd, recv_buf, recv_len, 0,
      (sockaddr *) &src_addr, src_addr_len);  // NOLINT

    ASSERT_EQ(send_len, recv_len);

    close(fd);
  });

  // Client that tests the echo server.
  exe.add([&]() {
    int fd = socket(SOCK_DGRAM);
    auto server_addr = addr(port);
    char send_buf[UDP_MESSAGE_SIZE];
    for (size_t i = 0; i < sizeof(send_buf); i++) {
      send_buf[i] = d(e);
    }

    io.sleep(fd, Io::Event::Write);

    auto send_len = ::sendto(fd, send_buf, sizeof(send_buf), 0,
      (sockaddr *) &server_addr, sizeof(server_addr));  // NOLINT

    ASSERT_EQ(send_len, sizeof(send_buf));

    io.sleep(fd, Io::Event::Read);

    sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    char recv_buf[sizeof(send_buf) + 1];
    auto recv_len = ::recvfrom(fd, recv_buf, sizeof(recv_buf), 0,
      (sockaddr *) &src_addr, &src_addr_len);  // NOLINT

    ASSERT_EQ(recv_len, sizeof(send_buf));
    ASSERT_EQ(std::memcmp(send_buf, recv_buf, sizeof(send_buf)), 0);
    ASSERT_EQ(src_addr.sin_addr.s_addr, server_addr.sin_addr.s_addr);
    ASSERT_EQ(src_addr.sin_port, server_addr.sin_port);

    close(fd);
  });

  exe.run();
}

}

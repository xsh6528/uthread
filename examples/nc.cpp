#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_string(host, "127.0.0.1", "The IPv4 address of the host.");

DEFINE_uint32(port, 8000, "The port to connect to on the host.");

/** An asynchronous IO service. */
static uthread::Io kIo;

/** File descriptor for the TCP connection. */
static int kTcpFd;

/**
 * Sends a stream of data from stdin to the endpoint.
 */
static void send_sock() {
  char buf[1024];

  kIo.sleep_on_fd(kTcpFd, uthread::Io::Event::Write);

  while (true) {
    kIo.sleep_on_fd(0, uthread::Io::Event::Read);

    int r = ::read(0, buf, sizeof(buf));
    if (r == -1) {
      LOG(ERROR) << "Stdin read: " << ::strerror(errno);
      ::exit(1);
    }

    int i = 0;
    while (i < r) {
      kIo.sleep_on_fd(kTcpFd, uthread::Io::Event::Write);

      int w = ::write(kTcpFd, buf + i, r - i);
      if (w == -1) {
        LOG(ERROR) << "Stdin write: " << ::strerror(errno);
        ::exit(1);
      }
      i += w;
    }
  }
}

/**
 * Prints the inbound TCP stream to stdout.
 */
static void recv_sock() {
  char buf[1024];

  while (true) {
    kIo.sleep_on_fd(kTcpFd, uthread::Io::Event::Read);

    int r = ::read(kTcpFd, buf, sizeof(buf));
    if (r == -1) {
      LOG(ERROR) << "TCP read: " << ::strerror(errno);
      ::exit(1);
    } else if (r == 0) {
      std::cout << "Connection closed." << std::endl;
      ::exit(0);
    }

    kIo.sleep_on_fd(1, uthread::Io::Event::Write);

    int w = ::write(1, buf, r);
    CHECK_EQ(w, r);
  }
}

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A stripped down (TCP-only) version of netcat.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  kTcpFd = uthread::nix::socket(SOCK_STREAM);
  if (kTcpFd == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  auto server_addr = uthread::nix::socket_addr(FLAGS_host, FLAGS_port);
  CHECK_NE(::connect(kTcpFd, (sockaddr *) &server_addr, sizeof(server_addr)),
    -1) << "Connection to endpoint has failed!";
  CHECK_NE(::uthread::nix::set_non_blocking(kTcpFd), -1);

  uthread::Executor exe;
  kIo.add(&exe);

  exe.add(send_sock);
  exe.add(recv_sock);

  exe.run();

  ::close(kTcpFd);

  return 0;
}

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_string(host, "127.0.0.1", "The IPv4 address of the host.");

DEFINE_uint32(port, 8000, "The port to connect to on the host.");

/** An asynchronous IO service. */
static uthread::nix::Io kIo;

/** File descriptor for the TCP connection. */
static int kTcpFd;

/**
 * Sends a stream of data from stdin to the endpoint.
 */
static void send_worker() {
  char buf[1024];

  while (true) {
    auto r = kIo.read(::uthread::nix::kStdinFd, buf, sizeof(buf));
    CHECK_GE(r, 0);

    if (kIo.send_s(kTcpFd, buf, r) == -1) {
      return;
    }
  }
}

/**
 * Prints the inbound TCP stream to stdout.
 */
static void recv_worker() {
  char buf[1024];

  while (true) {
    auto r = kIo.read(kTcpFd, buf, sizeof(buf));
    if (r == 0) {
      std::cout << "Connection closed." << std::endl;
      ::exit(0);
    } else if (r == -1) {
      LOG(ERROR) << "TCP read: " << ::strerror(errno);
      ::exit(1);
    }

    CHECK_GE(kIo.send_f(::uthread::nix::kStdoutFd, buf, r), 0);
  }
}

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A stripped down (TCP-only) version of netcat.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::cout << "Use 'ncat -l -t -k -p "
            << FLAGS_port
            << " -e /bin/cat' to run an "
            << "echo server."
            << std::endl;

  if ((kTcpFd = uthread::nix::socket(SOCK_STREAM)) == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  auto server_addr = uthread::nix::socket_addr(FLAGS_host, FLAGS_port);
  CHECK_NE(::connect(kTcpFd, (sockaddr *) &server_addr, sizeof(server_addr)),
    -1) << "Connection to endpoint has failed!";
  CHECK_NE(::uthread::nix::set_non_blocking(kTcpFd), -1);
  CHECK_NE(::uthread::nix::set_non_blocking(::uthread::nix::kStdinFd), -1);
  CHECK_NE(::uthread::nix::set_non_blocking(::uthread::nix::kStdoutFd), -1);

  uthread::Executor exe;
  kIo.add(&exe);

  exe.add(send_worker);
  exe.add(recv_worker);

  exe.run();

  ::close(kTcpFd);

  return 0;
}

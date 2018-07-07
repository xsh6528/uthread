#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_uint32(threads, 1, "The number of connection/worker threads to spawn.");

DEFINE_uint32(port, 8000, "The TCP port number to run the server on.");

/** An asynchronous IO service. */
static uthread::nix::Io kIo;

/** A queue of accepted connections for which workers echo. */
static uthread::MpmcQueue<int> kConn { 1 };

/**
 * Waits for accepted TCP connections in kConn and echo's inbound data.
 *
 * The worker processes exactly one connection at a time so more workers need
 * to be spawned to handle more concurrent connections.
 */
static void worker() {
  char buf[1024];

  while (true) {
    int fd;
    kConn.pop(fd);

    while (true) {
      auto r = kIo.read(fd, buf, sizeof(buf));
      if (r == -1 || kIo.send_s(fd, buf, r) == -1) break;
    }

    ::close(fd);
  }
}

/**
 * Starts a TCP echo server and pushes accepted connections to kConn.
 */
static void run() {
  int fd;

  if ((fd = uthread::nix::listener("127.0.0.1", FLAGS_port)) == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  while (true) {
    int conn = -1;
    while (conn == -1) {
      kIo.sleep_on_fd(fd, uthread::Io::Event::Read);
      conn = ::accept(fd, nullptr, nullptr);
    }
    kConn.push(conn);
  }
}

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A TCP echo server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::cout << "Running TCP echo server;  Use 'ncat 127.0.0.1 "
            << FLAGS_port
            << "' to send messages."
            << std::endl;

  uthread::Executor exe;
  kIo.add(&exe);

  for (auto i = FLAGS_threads; i > 0; i--) {
    exe.add(worker);
  }

  exe.add(run);

  exe.run();

  return 0;
}

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_uint32(threads, 1, "The number of connection/worker threads to spawn.");

DEFINE_uint32(port, 8000, "The TCP port number to run the server on.");

/** An asynchronous IO service. */
static uthread::Io kIo;

/** A queue of accepted connections for which workers echo. */
static uthread::MpmcQueue<int> kConn;

/**
 * Echo's the data from a single read(...) back to the sender.
 *
 * Returns -1 if an error occurred (eg. connection closed) and 0 otherwise.
 */
static int echorw(int fd) {
  char buf[2048];

  kIo.sleep_on_fd(fd, uthread::Io::Event::Read);

  int r = ::read(fd, buf, sizeof(buf));
  if (r == 0) {
    return -1;
  } else if (r == -1) {
    return 0;
  }

  int i = 0;
  while (i < r) {
    kIo.sleep_on_fd(fd, uthread::Io::Event::Write);

    int w = ::write(fd, buf + i, r - i);
    if (w == -1) {
      return -1;
    }
    i += w;
  }

  return 0;
}

/**
 * Waits for accepted TCP connections in kConn and echo's inbound data.
 *
 * The worker processes exactly one connection at a time so more workers need
 * to be spawned to handle more concurrent connections.
 */
static void worker() {
  while (true) {
    int fd;
    kConn.pop(fd);

    while (echorw(fd) != -1) {
    }

    ::close(fd);
  }
}

/**
 * Starts a TCP echo server and pushes accepted connections to kConn.
 */
static void listener() {
  LOG(INFO) << "Starting server...";

  int fd = uthread::nix::socket(SOCK_STREAM);
  if (fd == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  CHECK_NE(::uthread::nix::set_non_blocking(fd), -1);

  auto server_addr = uthread::nix::socket_addr("127.0.0.1", FLAGS_port);

  if (::bind(fd, (sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  if (::listen(fd, 16) == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  LOG(INFO) << "Running TCP echo server;  Use 'nc 127.0.0.1 "
            << FLAGS_port
            << "' to send messages.";

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

  uthread::Executor exe;
  kIo.add(&exe);

  for (auto i = FLAGS_threads; i > 0; i--) {
    exe.add(worker);
  }

  exe.add(listener);

  exe.run();

  return 0;
}

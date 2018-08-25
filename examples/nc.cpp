#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_string(host, "127.0.0.1", "The IPv4 address of the host.");

DEFINE_uint32(port, 8000, "The port to connect to on the host.");

/** A TCP connection to the endpoint. */
static uthread::TcpStream gStream;

static void sender() {
  char buf[1024];

  while (true) {
    uthread::Io::get()->sleep_on_fd(STDIN_FILENO, uthread::Io::Event::Read);
    auto r = read(STDIN_FILENO, buf, sizeof(buf));

    if (r == -1) {
        std::cerr << "Error reading stdin." << std::endl;
        exit(1);
    }

    switch (gStream.send(buf, r)) {
      case uthread::TcpStream::kErrClosed:
        std::cerr << "TCP connection closed." << std::endl;
        exit(0);
      case uthread::TcpStream::kErrOs:
        std::cerr << "Error sending via TCP stream." << std::endl;
        exit(1);
    }
  }
}

static void reader() {
  char buf[1024];

  while (true) {
    auto r = gStream.recv(buf, sizeof(buf));
    switch (r) {
      case uthread::TcpStream::kErrClosed:
        std::cerr << "TCP connection closed." << std::endl;
        exit(0);
      case uthread::TcpStream::kErrOs:
        std::cerr << "Error receiving from TCP stream." << std::endl;
        exit(1);
    }

    std::cout << std::string(buf, r);
  }
}

static void run() {
  CHECK_NE(gStream.connect(FLAGS_host, FLAGS_port), -1)
    << "Error connecting to endpoint!";
  CHECK_NE(uthread::nonblocking(STDIN_FILENO), -1);
  uthread::Executor::get()->add(sender);
  uthread::Executor::get()->add(reader);
}

int main(int argc, char *argv[]) {
  gflags::SetUsageMessage("A stripped down (TCP-only) version of netcat.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::cout << "Use 'ncat -l -t -k -p "
            << FLAGS_port
            << " -e /bin/cat' to run an "
            << "echo server."
            << std::endl;

  uthread::Executor exe;
  uthread::Io io(&exe);
  exe.add(run);
  exe.run();

  return 0;
}

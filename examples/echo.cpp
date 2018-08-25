#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_uint32(threads, 1, "The number of connection/worker threads to spawn.");

DEFINE_uint32(port, 8000, "The TCP port number to run the server on.");

/** A queue of accepted connections which workers work on echo'ing. */
static uthread::MpmcQueue<uthread::TcpStream> gStreams { 1 };

static void worker() {
  char buf[1024];

  while (true) {
    uthread::TcpStream stream;
    gStreams.pop(stream);

    while (true) {
      auto r = stream.recv(buf, sizeof(buf));
      if (r <= 0 || stream.send(buf, r) != 0) break;
    }
  }
}

static void run() {
  uthread::TcpListener listener;
  CHECK_NE(listener.bind("127.0.0.1", FLAGS_port), -1)
    << "Error binding listener!";

  while (true) {
    uthread::TcpStream stream;
    if (listener.accept(stream) != 0)
      LOG(ERROR) << "Error accepting connection!";
    gStreams.push(std::move(stream));
  }
}

int main(int argc, char *argv[]) {
  gflags::SetUsageMessage("A TCP echo server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::cout << "Running TCP echo server; Use 'ncat 127.0.0.1 "
            << FLAGS_port
            << "' to send messages."
            << std::endl;

  uthread::Executor exe;
  uthread::Io io(&exe);
  for (auto i = FLAGS_threads; i > 0; i--) {
    exe.add(worker);
  }
  exe.add(run);
  exe.run();

  return 0;
}

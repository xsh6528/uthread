#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_uint32(port, 8000, "The TCP port number to run the server on.");

struct User {
  /** The name of this user. */
  std::string name;
  /** The sequence number of the next message we should send this user. */
  size_t seq;
  /** The TCP connection for this user. */
  std::shared_ptr<uthread::TcpStream> stream;

  User() {
    stream = std::make_shared<uthread::TcpStream>();
  }
};

struct Room {
  /** The message log which we do NOT truncate fo simplicity. */
  std::vector<std::string> log;
  /** The name allocator for new connections. */
  uthread::MpmcQueue<std::string> names;

  Room() {
    names.push("Anonymous Iguana");
    names.push("Anonymous Turtle");
    names.push("Anonymous Bear");
    names.push("Anonymous Whale");
    names.push("Anonymous Shark");
    names.push("Anonymous Elephant");
    names.push("Anonymous Wolf");
    names.push("Anonymous Moose");
  }
};

struct Framer {
  std::string buf;

  /** Appends a char sequence to the framer. */
  void append(const char *buf_, size_t buf_size) {
    buf.append(buf_, buf_size);
  }

  /** Checks if the framer has a line. */
  bool has() {
    auto p = buf.find('\n');
    return (p != std::string::npos);
  }

  /** Pops a line from the framer. */
  std::string pop() {
    auto p = buf.find('\n');
    CHECK_NE(p, std::string::npos);
    auto s = buf.substr(0, p);
    buf.erase(0, p + 1);
    return s;
  }
};

/** Our chat room. */
static Room gRoom;

static void post(const User &user, std::string message) {
  static const std::string kWhitespace = " \t\r\n";

  // http://www.toptip.ca/2010/03/trim-leading-or-trailing-white-spaces.html
  auto p = message.find_first_not_of(kWhitespace);
  message.erase(0, p);

  p = message.find_last_not_of(kWhitespace);
  if (p != std::string::npos)
    message.erase(p + 1);

  if (message.empty())
    return;

  gRoom.log.push_back(user.name + ": " + message + "\n");
}

static void worker(User user) {
  post(user, "Connected");

  auto ok = true;

  // Send the log to this user.
  auto send = uthread::Executor::get()->add([&]() {
    while (ok) {
      if (user.seq < gRoom.log.size()) {
        const std::string &mess = gRoom.log[user.seq];
        if (user.stream->send(mess.c_str(), mess.size()) == 0) {
          user.seq++;
        } else {
          ok = false;
        }
      } else {
        uthread::Executor::get()->yield();
      }
    }
  });

  // Append messages from this user to the log.
  auto recv = uthread::Executor::get()->add([&]() {
    Framer framer;
    char buf[1024];

    while (ok) {
      auto r = user.stream->recv(buf, sizeof(buf));
      if (r >= 0) {
        framer.append(buf, r);
      } else {
        ok = false;
      }
      while (framer.has()) {
        post(user, framer.pop());
      }
    }
  });

  send.join();
  recv.join();

  // Send a notification that the user has disconnected and reuse the name.
  post(user, "Disconnected");
  gRoom.names.push(user.name);
}

static void run() {
  uthread::TcpListener listener;
  CHECK_NE(listener.bind("127.0.0.1", FLAGS_port), -1)
    << "Error binding listener!";

  while (true) {
    // Wait for a name to become available. This is a primitive way of
    // limiting the number of users in our chat room.
    User user;
    gRoom.names.pop(user.name);

    // A name has been allocated, so now we just wait for a connection.
    if (listener.accept(*user.stream) != 0)
      LOG(FATAL) << "Error accepting connection!";

    user.seq = gRoom.log.size();
    uthread::Executor::get()->add([user]() { worker(user); });
  }
}

int main(int argc, char *argv[]) {
  gflags::SetUsageMessage("A TCP echo server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::cout << "Running a chat server; Use 'ncat 127.0.0.1 "
            << FLAGS_port
            << "' to send messages."
            << std::endl;

  uthread::Executor exe;
  uthread::Io io(&exe);
  exe.add(run);
  exe.run();

  return 0;
}

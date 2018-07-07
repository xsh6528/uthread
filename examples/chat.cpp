#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <uthread/uthread.hpp>

DEFINE_uint32(port, 8000, "The TCP port number to run the server on.");

struct User {
  /** Name of the user. */
  std::string name;
  /** The sequence number of the next message we should sent the user. */
  size_t seq;
  /** A file descriptor corresponding to this user's TCP connection. */
  int fd;
};

struct Room {
  /** A message log which we do NOT truncate fo simplicity. */
  std::vector<std::string> log;
  /** A name allocator for new connections. */
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

/** An asynchronous IO service. */
static uthread::nix::Io kIo;

/** Our chat room. */
static Room kRoom;

/**
 * Posts a non-empty message by the specified user to the chat room.
 */
static void post(const User &user, std::string s) {
  static const std::string kWhitespace = " \t\r\n";

  // http://www.toptip.ca/2010/03/trim-leading-or-trailing-white-spaces.html
  auto p = s.find_first_not_of(kWhitespace);
  s.erase(0, p);

  p = s.find_last_not_of(kWhitespace);
  if (p != std::string::npos)
    s.erase(p + 1);

  if (s.empty())
    return;

  kRoom.log.push_back(user.name + ": " + s + "\n");
}

/**
 * Works on serving a user connection.
 */
static void worker(User user) {
  post(user, "Connected");

  auto ok = true;

  // Send the log to this user.
  auto send = uthread::Executor::current()->add([&]() {
    while (ok) {
      if (user.seq < kRoom.log.size()) {
        const std::string &mess = kRoom.log[user.seq];
        if (kIo.send_s(user.fd, mess.c_str(), mess.size()) != -1) {
          user.seq++;
        } else {
          ok = false;
        }
      } else {
        uthread::Executor::current()->yield();
      }
    }
  });

  // Append messages from this user to the log.
  auto recv = uthread::Executor::current()->add([&]() {
    Framer framer;
    char buf[1024];

    while (ok) {
      auto r = kIo.read(user.fd, buf, sizeof(buf));
      if (r > 0) {
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
  ::close(user.fd);
  post(user, "Disconnected");

  // Use the name for another user!
  kRoom.names.push(std::move(user.name));
}

/**
 * Starts a TCP chat server.
 */
static void run() {
  int fd;

  if ((fd = uthread::nix::listener("127.0.0.1", FLAGS_port)) == -1) {
    LOG(ERROR) << ::strerror(errno);
    ::exit(1);
  }

  while (true) {
    // Wait for a name to become available. This is a primitive way of
    // limiting the number of users in our chat room.
    User user;
    user.fd = -1;
    kRoom.names.pop(user.name);

    // A name has been allocated, so now we just wait for a connection.
    while (user.fd == -1) {
      kIo.sleep_on_fd(fd, uthread::Io::Event::Read);
      user.fd = ::accept(fd, nullptr, nullptr);
    }

    user.seq = kRoom.log.size();

    uthread::Executor::current()->add([user]() {
      worker(user);
    });
  }
}

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A TCP echo server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::cout << "Running a chat server;  Use 'ncat 127.0.0.1 "
            << FLAGS_port
            << "' to send messages."
            << std::endl;

  uthread::Executor exe;
  kIo.add(&exe);

  exe.add(run);

  exe.run();

  return 0;
}

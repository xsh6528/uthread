#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <uthread/nix.hpp>

namespace uthread { namespace nix {

int socket(int type) {
  int fd = ::socket(AF_INET, type, 0);
  if (fd == -1) {
    return -1;
  }

  int opt;

  opt = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt,
    sizeof(opt)) == -1) {
    ::close(fd);
    return -1;
  }

// https://nwat.xyz/blog/2014/01/16/porting-msg_more-and-msg_nosigpipe-to-osx/
#ifndef MSG_NOSIGNAL
  opt = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (const void *) &opt,
    sizeof(opt)) == -1) {
    ::close(fd);
    return -1;
  }
#endif

  return fd;
}

int listener(const std::string &addr, int port) {
  int fd;

  if ((fd = socket(SOCK_STREAM)) == -1) {
    return-1;
  }

  if (set_non_blocking(fd) == -1) {
    ::close(fd);
    return -1;
  }

  auto server_addr = uthread::nix::socket_addr(addr, port);

  if (::bind(fd, (sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    ::close(fd);
    return -1;
  }

  if (::listen(fd, 16) == -1) {
    ::close(fd);
    return -1;
  }

  return fd;
}

int set_non_blocking(int fd) {
  int flags = ::fcntl(fd, F_GETFL);
  if (flags == -1) {
    ::close(fd);
    return -1;
  }

  if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    ::close(fd);
    return -1;
  }

  return 0;
}

sockaddr_in socket_addr(const std::string &addr, int port) {
  sockaddr_in server_addr;
  ::bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = addr.empty()
    ? htonl(INADDR_ANY)
    : ::inet_addr(addr.c_str());
  server_addr.sin_port = htons(port);
  return server_addr;
}

int Io::read(int fd, void *buf, int buf_size) {
  sleep_on_fd(fd, Event::Read);

  return ::read(fd, buf, buf_size);
}

int Io::send_f(int fd, const void *buf, int buf_size) {
  int i = 0;

  while (i < buf_size) {
    sleep_on_fd(fd, Event::Write);

    auto w = ::write(fd, static_cast<const char *>(buf) + i, buf_size - i);
    if (w == -1) {
      return -1;
    }
    i += w;
  }

  return buf_size;
}

int Io::send_s(int fd, const void *buf, int buf_size) {
  int i = 0;

  while (i < buf_size) {
    sleep_on_fd(fd, Event::Write);

// https://nwat.xyz/blog/2014/01/16/porting-msg_more-and-msg_nosigpipe-to-osx/
#ifdef MSG_NOSIGNAL
    int flags = MSG_NOSIGNAL;
#else
    int flags = 0;
#endif

    auto w = ::send(fd, static_cast<const char *>(buf) + i, buf_size - i, flags);  // NOLINT
    if (w == -1) {
      return -1;
    }
    i += w;
  }

  return buf_size;
}

}}

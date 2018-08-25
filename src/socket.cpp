#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <glog/logging.h>

#include <uthread/io.hpp>
#include <uthread/socket.hpp>

namespace uthread {

static sockaddr_in socket_addr(const std::string &addr, int port) {
  sockaddr_in server_addr;
  std::memset((char *) &server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = addr.empty()
    ? htonl(INADDR_ANY)
    : inet_addr(addr.c_str());
  server_addr.sin_port = htons(port);
  return server_addr;
}

/**
 * ------------------------------------------------------------
 *                           UniqueFd
 * ------------------------------------------------------------
 */

UniqueFd::UniqueFd(int fd): fd_(fd) {
  CHECK_GE(fd, 0);
}

UniqueFd::UniqueFd(UniqueFd &&other) {
  fd_ = other.fd_;
  other.fd_ = -1;
}

UniqueFd& UniqueFd::operator=(UniqueFd &&other) {
  if (fd_ != -1)
    close(fd_);
  fd_ = other.fd_;
  other.fd_ = -1;
  return *this;
}

UniqueFd::~UniqueFd() {
  if (fd_ != -1)
    close(fd_);
}

int UniqueFd::operator*() const {
  return fd_;
}

/**
 * ------------------------------------------------------------
 *                           TcpStream
 * ------------------------------------------------------------
 */

int TcpStream::recv(void *buf, int buf_size) {
  CHECK_GE(buf_size, 0);

  Io::get()->sleep_on_fd(*fd_, Io::Event::Read);
  auto r = ::recv(*fd_, buf, buf_size, 0);
  switch (r) {
    case 0:
      return kErrClosed;
    case -1:
      return kErrOs;
    default:
      return r;
  }
}

int TcpStream::send(const void *buf, int buf_size) {
  CHECK_GE(buf_size, 0);

  int p = 0;

  while (p < buf_size) {
    auto ev = Io::get()->sleep_on_fd(*fd_, Io::Event::ReadWrite);

    if (UTHREAD_IO_EVENT_WRITABLE(ev)) {
#ifdef MSG_NOSIGNAL
      // https://nwat.xyz/blog/2014/01/16/porting-msg_more-and-msg_nosigpipe-to-osx/
      int flags = MSG_NOSIGNAL;
#else
      int flags = 0;
#endif

      auto w = ::send(*fd_, static_cast<const char *>(buf) + p,
        buf_size - p, flags);
      if (w == -1)
        return (errno == EPIPE) ? kErrClosed : kErrOs;
      p += w;
    } else if (UTHREAD_IO_EVENT_READABLE(ev) &&
        ::recv(*fd_, &p, sizeof(p), MSG_PEEK | MSG_DONTWAIT) == 0) {
      // http://stefan.buettcher.org/cs/conn_closed.html
      return kErrClosed;
    }
  }

  return 0;
}

int TcpStream::connect(const std::string &addr, int port) {
  if (*fd_ != -1)
    return kErrOpen;

  int raw = socket(AF_INET, SOCK_STREAM, 0);
  if (raw == -1)
    return kErrOs;

  UniqueFd fd(raw);

// https://nwat.xyz/blog/2014/01/16/porting-msg_more-and-msg_nosigpipe-to-osx/
#ifndef MSG_NOSIGNAL
  int opt = 1;
  if (setsockopt(*fd, SOL_SOCKET, SO_NOSIGPIPE, (const void *) &opt,
    sizeof(opt)) == -1) {
    return kErrOs;
  }
#endif

  if (nonblocking(*fd) == -1)
    return kErrOs;

  auto endpoint = socket_addr(addr, port);

  while (true) {
    if (::connect(*fd, (sockaddr *) &endpoint, sizeof(endpoint)) == 0 ||
        errno == EISCONN) {
      break;
    } else if (!(errno == EINPROGRESS || errno == EALREADY)) {
      return kErrOs;
    }
    Io::get()->sleep_on_fd(*fd, Io::Event::Write);
  }

  this->fd_ = std::move(fd);
  return 0;
}

/**
 * ------------------------------------------------------------
 *                           TcpListener
 * ------------------------------------------------------------
 */

int TcpListener::accept(TcpStream &stream) {
  while (true) {
    Io::get()->sleep_on_fd(*fd_, Io::Event::Read);
    auto fd = ::accept(*fd_, nullptr, nullptr);
    if (fd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
      return kErrOs;
    } else if (fd != -1) {
      stream.fd_ = UniqueFd(fd);
      return 0;
    }
  }

  return -1;
}

int TcpListener::bind(
    const std::string &addr,
    int port,
    TcpBindOptions options) {
  if (*fd_ != -1)
    return kErrOpen;

  int raw = socket(AF_INET, SOCK_STREAM, 0);
  if (raw == -1)
    return kErrOs;

  UniqueFd fd(raw);

  if (nonblocking(*fd) == -1)
    return kErrOs;

  int opt = 1;
  if (options.allow_addr_reuse && setsockopt(*fd, SOL_SOCKET,
    SO_REUSEADDR, (const void *) &opt, sizeof(opt)) == -1) {
    return kErrOs;
  }

  auto endpoint = socket_addr(addr, port);
  if (::bind(*fd, (sockaddr *) &endpoint, sizeof(endpoint)) == -1)
    return kErrOs;

  if (listen(*fd, 16) == -1)
    return kErrOs;

  this->fd_ = std::move(fd);
  return 0;
}

int nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags == -1)
    return -1;

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    return -1;

  return 0;
}

}

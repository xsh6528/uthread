#include <string>

#ifndef UTHREAD_SOCKET_HPP_
#define UTHREAD_SOCKET_HPP_

namespace uthread {

/**
 * A unique ownership wrapper around a raw file descriptor.
 */
class UniqueFd {
 public:
  UniqueFd() = default;

  explicit UniqueFd(int fd);

  UniqueFd(const UniqueFd &) = delete;

  UniqueFd(UniqueFd &&);

  UniqueFd& operator=(const UniqueFd &) = delete;

  UniqueFd& operator=(UniqueFd &&);

  ~UniqueFd();

  int raw() const;

 private:
  int fd_ = -1;
};

class TcpListener;

/**
 * A TCP connection/stream.
 */
class TcpStream {
 public:
  static constexpr int kErrClosed = -1;

  static constexpr int kErrOpen = -2;

  static constexpr int kErrOs = -3;

  /**
   * Receives up to buf_size bytes into the buffer.
   *
   * Returns an error constant or the number of bytes read.
   */
  int recv(void *buf, int buf_size);

  /**
   * Sends buf_size bytes from the buffer.
   *
   * Returns an error constant or the number of bytes send.
   */
  int send(const void *buf, int buf_size);

  /**
   * Connects to an endpoint.
   *
   * Returns an error constant or 0 on success.
   */
  int connect(const std::string &addr, int port);

 private:
  UniqueFd fd_;

  friend class TcpListener;
};

/**
 * A TCP listener for accepting streams/connections.
 */
class TcpListener {
 public:
  static constexpr int kErrOpen = -1;

  static constexpr int kErrOs = -2;

  /**
   * Accepts a connection.
   *
   * Returns an error constant or 0 on success.
   */
  int accept(TcpStream &stream);

  /**
   * Begins listening for connections.
   *
   * Returns an error constant or 0 on success.
   */
  int bind(const std::string &addr, int port);

 private:
  UniqueFd fd_;
};

/**
 * Enables non-blocking reads and writes on a file descriptor.
 */
int set_async(int fd);

}

#endif  // UTHREAD_SOCKET_HPP_

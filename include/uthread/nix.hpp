#include <string>
#include <sys/socket.h>

#include <uthread/io.hpp>

#ifndef UTHREAD_NIX_HPP_
#define UTHREAD_NIX_HPP_

namespace uthread { namespace nix {

static constexpr int kStdinFd = 0;

static constexpr int kStdoutFd = 1;

/**
 * Creates a non-blocking IPv4 socket of one of the types listed here:
 *
 * http://man7.org/linux/man-pages/man2/socket.2.html
 */
int socket(int type);

/**
 * Creates a non-blocking TCP listener with the specified address and port.
 */
int listener(const std::string &addr, int port);

/**
 * Makes a socket non-blocking.
 */
int set_non_blocking(int fd);

/**
 * Creates a socket address from an IPv4 address and port.
 */
sockaddr_in socket_addr(const std::string &addr, int port);

/**
 * An extension to the barebones IO class with some useful bits of code that
 * should work on *nix OS flavors.
 */
class Io : public ::uthread::Io {
 public:
  /**
   * Reads up to buf_size bytes into the buffer from a file or socket.
   *
   * Returns the number of bytes read on success and -1 in case of an error.
   */
  int read(int fd, void *buf, int buf_size);

  /**
   * Writes the entire buffer to a file.
   *
   * Returns the number of bytes written on success and -1 in case of an error.
   */
  int send_f(int fd, const void *buf, int buf_size);

  /**
   * Writes the entire buffer to a socket. Use this function when sending data
   * via sockets for protection against SIG_PIPE!
   *
   * Returns the number of bytes written on success and -1 in case of an error.
   */
  int send_s(int fd, const void *buf, int buf_size);
};

}}

#endif  // UTHREAD_NIX_HPP_

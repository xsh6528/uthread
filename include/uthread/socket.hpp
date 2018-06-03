#include <string>
#include <sys/socket.h>

#ifndef UTHREAD_SOCKET_HPP_
#define UTHREAD_SOCKET_HPP_

namespace uthread { namespace nix {

/**
 * Creates a non-blocking IPv4 socket of one of the types listed here:
 *
 * http://man7.org/linux/man-pages/man2/socket.2.html
 */
int socket(int type);

/**
 * Creates a socket address from an IPv4 address and port.
 */
sockaddr_in socket_addr(const std::string &addr, int port);

}}

#endif  // UTHREAD_SOCKET_HPP_

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <uthread/socket.hpp>

namespace uthread { namespace nix {

int socket(int type) {
  int fd = ::socket(AF_INET, type, 0);
  if (fd == -1) {
    return -1;
  }

  int opt = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt,
    sizeof(opt)) == -1) {
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

}}

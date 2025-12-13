#include "socket.h"
#include "logging.h"
#include "protocal.h"
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

namespace local_transport {

Socket::Socket(int port) : Socket("", port) {}

Socket::Socket(const std::string &remote_ip, int port) {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ == -1) {
    LOG << "Failed to create socket";
    std::abort();
  }
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr =
      remote_ip.empty() ? INADDR_ANY : inet_addr(remote_ip.c_str());
  addr_.sin_port = htons(port);
}

Socket::~Socket() { close(socket_fd_); }

int Socket::Listen() {
  int optval = 1;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                 sizeof(optval)) == -1) {
    LOG << "Failed to set socket option, " << strerror(errno)
        << ", socket_fd_: " << socket_fd_;
    std::abort();
  }
  if (bind(socket_fd_, (struct sockaddr *)&addr_, sizeof(addr_)) == -1) {
    LOG << "Failed to bind socket, " << strerror(errno)
        << ", socket_fd_: " << socket_fd_;
    std::abort();
  }
  if (::listen(socket_fd_, 10) == -1) {
    LOG << "Failed to listen socket";
    std::abort();
  }
  return socket_fd_;
}

int Socket::Accept() {
  int connfd = ::accept(socket_fd_, nullptr, nullptr);
  if (connfd == -1) {
    LOG << "Failed to accept connection, " << strerror(errno);
    std::abort();
  }
  return connfd;
}

// Caller shall close the socket.
int Socket::Connect() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    LOG << "Failed to create socket, " << strerror(errno);
    std::abort();
  }
  if (::connect(socket_fd, (struct sockaddr *)&addr_, sizeof(addr_)) == -1) {
    LOG << "Failed to connect socket, " << strerror(errno)
        << ", socket_fd_: " << socket_fd;
    std::abort();
  }
  return socket_fd;
}

} // namespace local_transport

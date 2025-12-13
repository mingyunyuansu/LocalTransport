#pragma once

#include "protocal.h"
#include <netinet/in.h>
#include <string>

namespace local_transport {

// You may only call one of Listen() and Connect().
class Socket {
public:
  Socket(int port);
  Socket(const std::string &remote_ip, int port);

  ~Socket();

  // Return the listener fd.
  int Listen();
  int Accept();
  // Return the connection fd.
  int Connect();

private:
  sockaddr_in addr_;
  int socket_fd_ = -1;
};

} // namespace local_transport

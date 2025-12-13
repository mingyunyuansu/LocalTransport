#pragma once

#include "protocal.h"
#include "socket.h"

namespace local_transport {

class Receiver {
public:
  Receiver(int port = kPort);
  ~Receiver();

  bool Init();

  bool Recv();

private:
  bool RecvHandShakeMsg();

  bool RecvOneSpan();

  Socket socket_;
  int filename_size_ = -1;
  size_t file_size_ = -1;
  std::string filename_;
  int span_num_ = -1;

  void* file_content_ = nullptr;
};

} // namespace local_transport

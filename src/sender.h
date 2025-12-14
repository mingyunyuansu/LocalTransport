#pragma once

#include "protocal.h"
#include "socket.h"
#include <atomic>
#include <string>

namespace local_transport {

class Sender {
public:
  Sender(const std::string &filepath, const std::string &remote_ip,
         int port = kPort);

  bool Init();

  bool Send();

private:
  HandShakeCtx PackHandShakeMsg();

  bool SendOneSpan(int span_idx, size_t size);

  std::string filepath_;
  size_t file_size_;
  std::string filename_;
  Socket socket_;
  void *file_content_;
  int span_num_;
  std::atomic<size_t> total_sent_{0};
};

} // namespace local_transport

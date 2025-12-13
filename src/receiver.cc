#include "receiver.h"
#include "logging.h"
#include "protocal.h"
#include "socket.h"
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace local_transport {

Receiver::Receiver(int port) : socket_(port) {}

Receiver::~Receiver() {
  if (file_content_ != nullptr) {
    munmap(file_content_, file_size_);
  }
}

bool Receiver::Init() { return socket_.Listen() >= 0; }

bool Receiver::Recv() {
  if (!RecvHandShakeMsg()) {
    LOG << "recv hand shake msg failed";
    std::abort();
  }
  int local_fd = open(filename_.c_str(), O_RDWR | O_CREAT, 0644);
  if (local_fd < 0) {
    LOG << "open file failed, errno: " << errno;
    std::abort();
  }
  int r = ftruncate(local_fd, file_size_);
  if (r < 0) {
    LOG << "truncate file failed, errno: " << strerror(errno);
    std::abort();
  }
  file_content_ = mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED,
                       local_fd, 0);
  if (file_content_ == MAP_FAILED) {
    LOG << "mmap file failed, errno: " << strerror(errno)
        << ", file_size: " << file_size_;
    std::abort();
  }
  close(local_fd);
  std::vector<std::thread> threads;
  for (int i = 0; i < span_num_; ++i) {
    threads.emplace_back([this]() { RecvOneSpan(); });
  }
  for (auto &thread : threads) {
    thread.join();
  }
  return true;
}

bool Receiver::RecvHandShakeMsg() {
  int connfd = socket_.Accept();
  if (connfd < 0) {
    LOG << "accept connection failed, errno: " << errno;
    std::abort();
  }
  int n = 0;
  char buf[kHandShakeCtxSize];
  while (n < kHandShakeCtxSize) {
    int ret = ::recv(connfd, buf + n, kHandShakeCtxSize - n, 0);
    if (ret == -1) {
      LOG << "recv failed, errno: " << errno;
      std::abort();
    }
    n += ret;
  }
  auto *msg = reinterpret_cast<HandShakeCtx *>(buf);
  if (msg->type != HandShakeType::kHandShake) {
    LOG << "invalid hand shake type: " << static_cast<int>(msg->type);
    std::abort();
  }
  filename_size_ = msg->content.msg.filename_size;
  file_size_ = msg->content.msg.file_size;
  span_num_ = msg->content.msg.span_num;
  filename_.resize(filename_size_);
  memcpy(filename_.data(), msg->content.msg.filename, filename_size_);
  return true;
}

bool Receiver::RecvOneSpan() {
  int connfd = socket_.Accept();
  if (connfd < 0) {
    LOG << "accept connection failed, errno: " << errno;
    std::abort();
  }
  int n = 0;
  char buf[kHandShakeCtxSize];
  while (n < kHandShakeCtxSize) {
    int ret = ::recv(connfd, buf + n, kHandShakeCtxSize - n, 0);
    if (ret == -1) {
      LOG << "recv failed, errno: " << errno;
      std::abort();
    }
    n += ret;
  }
  auto *span_msg = reinterpret_cast<HandShakeCtx *>(buf);
  if (span_msg->type != HandShakeType::kSpanHandShake) {
    LOG << "invalid span hand shake type: " << static_cast<int>(span_msg->type);
    std::abort();
  }
  LOG << "recv span hand shake ctx: " << *span_msg;
  int span_idx = span_msg->content.span_msg.span_id;
  size_t offset = span_msg->content.span_msg.offset;
  size_t size = span_msg->content.span_msg.size;
  n = 0;
  while (n < size) {
    int ret =
        ::recv(connfd, reinterpret_cast<char *>(file_content_) + offset + n,
               size - n, 0);
    if (ret == -1) {
      LOG << "recv failed, errno: " << errno;
      std::abort();
    }
    n += ret;
  }
  LOG << "Write span " << span_idx << " done, offset: " << offset
      << ", size: " << size << ", n: " << n;
  return true;
}

} // namespace local_transport

#include "sender.h"
#include "logging.h"
#include "protocal.h"

#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace local_transport {

Sender::Sender(const std::string &filepath, const std::string &remote_ip,
               int port)
    : filepath_(filepath), file_size_(0), socket_(remote_ip, port),
      file_content_(nullptr), span_num_(0) {}

// TODO: handle fail case.
bool Sender::Init() {
  int file_fd = open(filepath_.c_str(), O_RDONLY);
  if (file_fd < 0) {
    LOG << "open file " << filepath_ << " failed, errno: " << errno;
    std::abort();
  }
  struct stat file_stat = {};
  if (fstat(file_fd, &file_stat) < 0) {
    LOG << "fstat file " << filepath_ << " failed, errno: " << errno;
    std::abort();
  }
  std::string filename = filepath_.substr(filepath_.rfind('/') + 1);
  LOG << "file: " << filename << " size: " << file_stat.st_size;
  filename_ = filename;
  file_size_ = file_stat.st_size;
  file_content_ =
      mmap(nullptr, file_stat.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
  if (file_content_ == MAP_FAILED) {
    LOG << "mmap file " << filepath_ << " failed, errno: " << errno;
    std::abort();
  }
  span_num_ = (file_size_ + kSpanSize - 1) / kSpanSize;
  return true;
}

bool Sender::Send() {
  auto now = std::chrono::steady_clock::now();
  HandShakeCtx msg = PackHandShakeMsg();
  // Handshake first for file meta.
  int connfd = socket_.Connect();
  int sent = 0;
  while (sent < kHandShakeCtxSize) {
    int n = write(connfd, &msg, kHandShakeCtxSize - sent);
    if (n < 0) {
      // TODO: handle failure.
      LOG << "write to server failed, errno: " << strerror(errno);
      std::abort();
    }
    sent += n;
  }
  // Send each span.
  std::vector<std::thread> threads;
  for (int i = 0; i < span_num_; ++i) {
    size_t span_size = std::min(kSpanSize, file_size_ - i * kSpanSize);
    threads.emplace_back([this, i, span_size]() { SendOneSpan(i, span_size); });
  }
  std::thread progress_thread([this]() {
    // Log progress every second.
    while (total_sent_ < file_size_) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      double progress = (double)total_sent_ / file_size_;
      LOG << "Send progress: " << total_sent_ << "/" << file_size_ << " ("
          << progress << "%)";
    }
  });
  for (auto &thread : threads) {
    thread.join();
  }
  progress_thread.join();
  auto end = std::chrono::steady_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - now);
  LOG << "Send file " << filename_ << " done, span num: " << span_num_
      << ", filesize: " << file_size_ << ", time cost: " << duration.count()
      << " ms"
      << ", speed: " << (double)file_size_ / duration.count() / 1024 << " MB/s";
  return true;
}

bool Sender::SendOneSpan(int span_idx, size_t size) {
  HandShakeCtx msg = {};
  msg.type = HandShakeType::kSpanHandShake;
  msg.content.span_msg.span_id = span_idx;
  msg.content.span_msg.offset = span_idx * kSpanSize;
  msg.content.span_msg.size = size;
  int connfd = socket_.Connect();
  // Span handshake.
  int sent = 0;
  while (sent < kHandShakeCtxSize) {
    int n = write(connfd, &msg, kHandShakeCtxSize - sent);
    if (n < 0) {
      // TODO: handle failure.
      LOG << "write to server failed, errno: " << strerror(errno);
      std::abort();
    }
    sent += n;
  }
  // Span data.write to server failed
  sent = 0;
  LOG << "Try send span: " << msg.content.span_msg;
  while (sent < size) {
    int n = write(connfd,
                  (char *)file_content_ + msg.content.span_msg.offset + sent,
                  size - sent);
    if (n < 0) {
      // TODO: handle failure.
      LOG << "write to server failed, errno: " << strerror(errno);
      std::abort();
    }
    sent += n;
    total_sent_ += n;
  }
  close(connfd);
  return true;
}

HandShakeCtx Sender::PackHandShakeMsg() {
  HandShakeCtx msg = {};
  msg.type = HandShakeType::kHandShake;
  msg.content.msg.filename_size = filename_.size();
  msg.content.msg.file_size = file_size_;
  memcpy(msg.content.msg.filename, filename_.c_str(), filename_.size());
  msg.content.msg.span_num = span_num_;
  return msg;
}

} // namespace local_transport

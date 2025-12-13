#include "logging.h"
#include "sender.h"

int main() {
  const std::string filepath =
      "/mnt/c/迅雷下载/"
      "【高清MP4电影www.mp4kan.com】plmxs.2012.BD1080p.zysz.mp4";
  local_transport::Sender sender(filepath, "192.168.71.32");
  if (!sender.Init()) {
    LOG << "sender init failed";
    std::abort();
  }
  LOG << "sender init success";
  if (!sender.Send()) {
    LOG << "sender send failed";
    std::abort();
  }
}
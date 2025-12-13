#include "logging.h"
#include "receiver.h"

int main() {
  local_transport::Receiver receiver;
  if (!receiver.Init()) {
    LOG << "receiver init failed";
    std::abort();
  }
  LOG << "receiver init success";
  if (!receiver.Recv()) {
    LOG << "receiver recv failed";
    std::abort();
  }
}
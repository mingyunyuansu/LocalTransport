#pragma once

#include <ostream>
const int kPort = 54321;
const int kMaxFileNameLen = 256;
const size_t kSpanSize = 256 * 1024 * 1024;

enum class HandShakeType { kHandShake = 0, kSpanHandShake = 1 };

struct HandShakeMsg {
  int filename_size;
  size_t file_size;
  char filename[kMaxFileNameLen];
  int span_num;
};
inline std::ostream &operator<<(std::ostream &os, const HandShakeMsg &msg) {
  os << "filename_size: " << msg.filename_size
     << ", file_size: " << msg.file_size << ", filename: " << msg.filename
     << ", span_num: " << msg.span_num;
  return os;
}

struct SpanHandShakeMsg {
  int span_id;
  size_t offset;
  size_t size;
};
inline std::ostream &operator<<(std::ostream &os, const SpanHandShakeMsg &msg) {
  os << "span_id: " << msg.span_id << ", offset: " << msg.offset
     << ", size: " << msg.size;
  return os;
}

struct HandShakeCtx {
  HandShakeType type;
  union {
    HandShakeMsg msg;
    SpanHandShakeMsg span_msg;
  } content;
};

inline std::ostream &operator<<(std::ostream &os, const HandShakeCtx &ctx) {
  std::string type_str =
      ctx.type == HandShakeType::kHandShake ? "kHandShake" : "kSpanHandShake";
  os << "type: " << type_str << ", content: ";
  if (ctx.type == HandShakeType::kHandShake) {
    os << ctx.content.msg;
  } else {
    os << ctx.content.span_msg;
  }
  return os;
}

const int kHandShakeCtxSize = sizeof(HandShakeCtx);

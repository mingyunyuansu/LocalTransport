#pragma once

#include <iostream>
#include <sstream>

class Logger {
public:
  Logger() = default;
  ~Logger() {
    oss_ << std::endl;
    std::cout << oss_.str();
  }

  auto &stream(const char *file, int line) {
    oss_ << file << ":" << line << ": ";
    return oss_;
  }

private:
  std::stringstream oss_;
};

#define LOG Logger().stream(__FILE__, __LINE__)

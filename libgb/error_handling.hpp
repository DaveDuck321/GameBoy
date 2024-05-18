#pragma once

#include <stdexcept>
#include <string>

namespace gb {
class IllegalMemoryRead : public std::runtime_error {
 public:
  explicit IllegalMemoryRead(const std::string& msg)
      : std::runtime_error(msg) {}
};

class BadOpcode : public std::runtime_error {
 public:
  explicit BadOpcode(const std::string& msg) : std::runtime_error(msg) {}
};

}  // namespace gb

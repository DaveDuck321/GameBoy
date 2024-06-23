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

class Trap : public BadOpcode {
 public:
  using BadOpcode::BadOpcode;
};

class CorrectnessError : public std::runtime_error {
 public:
  explicit CorrectnessError(const std::string& msg) : std::runtime_error(msg) {}
};

class UndefinedDataError : public CorrectnessError {
 public:
  using CorrectnessError::CorrectnessError;
};

class CallFrameViolationError : public CorrectnessError {
 public:
  using CorrectnessError::CorrectnessError;
};

class ClobberedReturnAddressError : public CorrectnessError {
 public:
  using CorrectnessError::CorrectnessError;
};

}  // namespace gb

#pragma once

#include <array>
#include <stdexcept>
#include <string>

namespace gb {

enum class ErrorKind {
  bad_opcode,
  trap,
  debug_trap,
  illegal_memory_address,
  illegal_memory_write,
  undefined_data,
  call_frame_violation,
  clobbered_return_address,
  reading_return_address,
  _last,
};
static constexpr auto error_kind_count = static_cast<size_t>(ErrorKind::_last);

class BadOpcode : public std::runtime_error {
 public:
  static constexpr auto kind = ErrorKind::bad_opcode;

  explicit BadOpcode(const std::string& msg) : std::runtime_error(msg) {}
};

class Trap : public BadOpcode {
 public:
  static constexpr auto kind = ErrorKind::trap;

  using BadOpcode::BadOpcode;
};

class DebugTrap : public Trap {
 public:
  static constexpr auto kind = ErrorKind::debug_trap;

  using Trap::Trap;
};

class CorrectnessError : public std::runtime_error {
 public:
  explicit CorrectnessError(const std::string& msg) : std::runtime_error(msg) {}
};

class IllegalMemoryAddress : public std::runtime_error {
 public:
  static constexpr auto kind = ErrorKind::illegal_memory_address;

  explicit IllegalMemoryAddress(const std::string& msg)
      : std::runtime_error(msg) {}
};

class IllegalMemoryWrite : public std::runtime_error {
 public:
  static constexpr auto kind = ErrorKind::illegal_memory_write;

  explicit IllegalMemoryWrite(const std::string& msg)
      : std::runtime_error(msg) {}
};

class UndefinedDataError : public CorrectnessError {
 public:
  static constexpr auto kind = ErrorKind::undefined_data;

  using CorrectnessError::CorrectnessError;
};

class CallFrameViolationError : public CorrectnessError {
 public:
  static constexpr auto kind = ErrorKind::call_frame_violation;

  using CorrectnessError::CorrectnessError;
};

class ClobberedReturnAddressError : public CorrectnessError {
 public:
  static constexpr auto kind = ErrorKind::clobbered_return_address;

  using CorrectnessError::CorrectnessError;
};

class ReadingReturnAddressError : public CorrectnessError {
 public:
  static constexpr auto kind = ErrorKind::reading_return_address;

  using CorrectnessError::CorrectnessError;
};

extern std::array<unsigned, error_kind_count> error_count;
extern std::array<bool, error_kind_count> error_kind_permitted;

template <typename Fn>
static auto throw_error(Fn&& get_error) -> void {
  using ErrorT = decltype(get_error());
  constexpr auto kind = static_cast<size_t>(ErrorT::kind);

  error_count[kind] += 1;
  if (not error_kind_permitted[kind]) {
    throw get_error();
  }
}

auto permit_error_kind(ErrorKind kind) -> void;

}  // namespace gb

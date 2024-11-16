#include "error_handling.hpp"

#include <array>

namespace gb {
std::array<unsigned, error_kind_count> error_count = {};
std::array<bool, error_kind_count> error_kind_permitted = {};

auto permit_error_kind(ErrorKind kind) -> void {
  error_kind_permitted[static_cast<size_t>(kind)] = true;
}
}  // namespace gb

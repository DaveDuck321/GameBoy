#pragma once

#include "../utils/checked_int.hpp"

#include <cstdint>

namespace gb {
class Controller {
 public:
  Controller() = default;
  Controller(const Controller&) = delete;
  auto operator=(const Controller&) -> Controller& = delete;
  virtual ~Controller() = default;

  [[nodiscard]] virtual auto read(uint16_t addr) const -> Byte = 0;
  virtual void write(uint16_t addr, Byte value) = 0;
};
}  // namespace gb

#pragma once

#include <cstdint>

namespace gb {
class Controller {
 public:
  Controller() = default;
  Controller(const Controller&) = delete;
  auto operator=(const Controller&) -> Controller& = delete;
  virtual ~Controller() = default;

  [[nodiscard]] virtual auto read(uint16_t addr) const -> uint8_t = 0;
  virtual void write(uint16_t addr, uint8_t value) = 0;
};
}  // namespace gb

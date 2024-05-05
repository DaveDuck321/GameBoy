#include "controller.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <span>

using namespace gb;

class RomOnlyController : public Controller {
  std::span<uint8_t> rom;

 public:
  explicit RomOnlyController(std::span<uint8_t> rom) : rom{rom} {}

  [[nodiscard]] auto read(uint16_t addr) const -> uint8_t final {
    return rom[addr];
  }

  auto write(uint16_t addr, uint8_t value) -> void final {
    // ROM should just ignore write errors
    // Some games write to the controller even if there's just ROM
    // Print message for debugging anyway
    std::cerr << std::hex << "ROM write requested! Addr: " << addr
              << " Value: " << (int)value << std::endl;
  }
};

namespace gb {
auto make_rom_only_controller(std::span<uint8_t> rom)
    -> std::unique_ptr<Controller> {
  return std::make_unique<RomOnlyController>(rom);
}
}  // namespace gb

#include "../error_handling.hpp"
#include "controller.hpp"

#include <cstdint>
#include <format>
#include <memory>
#include <span>

using namespace gb;

class RomOnlyController : public Controller {
  std::span<uint8_t> rom;

 public:
  explicit RomOnlyController(std::span<uint8_t> rom) : rom{rom} {}

  [[nodiscard]] auto read(uint16_t addr) const -> Byte final {
    if (addr < rom.size()) {
      return Byte{rom[addr]};
    }
    return 0_B;  // Implicit zero pad
  }

  auto write(uint16_t addr, Byte value) -> void final {
    // ROM should just ignore write errors (when UBSAN is off)
    // Some games write to the controller even if there's just ROM
    throw_error([&] {
      return IllegalMemoryWrite(
          std::format("Attempt to write {:#04x} to read-only address @ {:#06x}",
                      value.decay(), addr));
    });
  }
};

namespace gb {
auto make_rom_only_controller(std::span<uint8_t> rom)
    -> std::unique_ptr<Controller> {
  return std::make_unique<RomOnlyController>(rom);
}
}  // namespace gb

#include "controller.hpp"

#include "../error_handling.hpp"

#include <array>
#include <cstdint>
#include <format>
#include <memory>
#include <span>

using namespace gb;

class MBC1 : public Controller {
  // Possible memory modes are
  //   16Mbit ROM/8KByte RAM   (false)
  //   4Mbit ROM/32KByte RAM   (true)
  bool bankedRamMode = false;
  bool ramEnabled = true;

  // Both RAM and ROM banks can be selected by index
  uint8_t romBank = 1;
  uint8_t ramBank = 0;

  std::span<uint8_t> rom;

  // Allocate enough ram for the full 32KByte RAM mode
  std::array<Byte, 0x8000> ram = {};

 public:
  explicit MBC1(std::span<uint8_t> rom) : rom{rom} {}

  [[nodiscard]] auto read(uint16_t addr) const -> Byte final {
    switch (addr >> 12U) {
      // Always read from ROM bank 0 if addr < 0x4000
      case 0:
      case 1:
      case 2:
      case 3:
        return Byte{rom[addr]};
      // Selectable rom banks from 0x4000 - 0x7FFF
      case 4:
      case 5:
      case 6:
      case 7: {
        uint16_t bankOffset = addr - 0x4000;
        return Byte{rom[(0x4000 * romBank) + bankOffset]};
      }
      // Cartridge RAM (Selectable in 32KByte RAM mode)
      case 0xA:
      case 0xB: {
        uint16_t bankOffset = addr - 0xA000;
        return ram.at((0x2000 * ramBank) + bankOffset);
      }
      default:
        throw_error([&] {
          return IllegalMemoryAddress(
              std::format("Cannot read ROM address {:#06x}", addr));
        });
        return {};
    }
  }

  auto write(uint16_t addr, Byte value) -> void final {
    switch (addr >> 12U) {
      // 0x0000 - 0x1FFF area disables RAM in 32KByte RAM mode
      case 0:
      case 1:
        if (!bankedRamMode) {
          break;
        }
        ramEnabled = (value & 0x0A_B) == 0xA_B;
        break;
      // 0x2000 - 0x3FFF area selects ROM bank
      case 2:
      case 3:
        if ((value & 0x1F_B) != 0_B) {
          romBank = (value & 0x1F_B).decay();
        } else {
          romBank = 1;  // Cannot select bank 0
        }
        break;

      // 0x4000 - 0x5FFF area selects either:
      //      ROM bank significant bits (in 16Mbit ROM)
      //      RAM bank                  (in 32KByte RAM mode)
      case 4:
      case 5:
        if (bankedRamMode) {
          ramBank = (value & 0x03_B).decay();
        } else {
          romBank =
              (((value & 0x03_B) << 5U) | (Byte{romBank} & 0x1F_B)).decay();
        }
        break;

      // 0x6000 - 0x7FFF area selects memory mode
      case 6:
      case 7:
        bankedRamMode = (value & 1_B) != 0_B;
        break;

      // 0xA000 - 0xBFFF area is the cartridge RAM.
      // This is a normal write unlike the other operations
      case 0xA:
      case 0xB: {
        uint16_t bankOffset = addr - 0xA000;
        ram[(0x2000 * ramBank) + bankOffset] = value;
        break;
      }

      default:
        throw_error([&] {
          return IllegalMemoryAddress(
              std::format("Cannot write to ROM address {:#06x}", addr));
        });
        break;
    }
  }
};

namespace gb {
auto make_mbc1(std::span<uint8_t> rom) -> std::unique_ptr<Controller> {
  return std::make_unique<MBC1>(rom);
}
}  // namespace gb

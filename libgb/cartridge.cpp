#include "cartridge.hpp"

#include <fstream>
#include <span>

using namespace gb;

auto Cartridge::loadFromRom(std::string_view name) -> Cartridge {
  std::ifstream input(std::string(name), std::ios::binary);
  if (!input) {
    throw std::runtime_error("Couldn't open ROM");
  }

  return Cartridge(
      std::vector<uint8_t>(std::istreambuf_iterator<char>(input), {}));
}

Cartridge::Cartridge(std::vector<uint8_t>&& rom_file)
    : rom{std::move(rom_file)} {
  populateMetadata(rom);

  // Deduce controller from rom
  switch (controllerType) {
    case 0:  // ROM only
      controller = make_rom_only_controller(rom);
      break;
    case 1:
    case 2:
    case 3:  // MBC1 Controller
      controller = make_mbc1(rom);
      break;
    default:
      throw std::runtime_error("Cartridge controller not implemented");
  }
}

void Cartridge::populateMetadata(const std::vector<uint8_t>& rom) {
  /*
  Populates useful cartridge information from the ROM.

  Magic numbers from here:
  http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
  */

  // GB Color bit
  target = Target{rom[0x143]};

  // Name in upper ASCII
  gameName = std::string(rom.begin() + 0x134, rom.begin() + 0x142);

  // Describes the cartridge technology used
  // This might be misrepresented by the game -- could cause errors later
  controllerType = rom[0x147];

  // ROM size and type -- enum type
  romSize = rom[0x148];

  // RAM size and type -- enum type
  ramSize = rom[0x149];
}

auto Cartridge::read(uint16_t addr) const -> uint8_t {
  return controller->read(addr);
}

void Cartridge::write(uint16_t addr, uint8_t value) {
  controller->write(addr, value);
}

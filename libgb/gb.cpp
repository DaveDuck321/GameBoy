#include "gb.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include "cartridge.hpp"

using namespace gb;

GB::GB(std::string_view rom_file, std::unique_ptr<IOFrontend> io_frontend)
    : cartridge(Cartridge::loadFromRom(rom_file)),
      io(std::move(io_frontend)),
      memory_map(cartridge, io),
      cpu(memory_map, io) {}

auto GB::readU8(uint16_t addr) const -> uint8_t {
  return memory_map.read(addr);
}

auto GB::readU16(uint16_t addr) const -> uint16_t {
  uint8_t lower = memory_map.read(addr);
  uint8_t upper = memory_map.read(addr + 1);
  return (upper << 8U) + lower;
}

auto GB::isSimulationFinished() -> bool {
  return io.isSimulationFinished();
}

auto GB::clock() -> void {
  // Update timers for accurate delays
  // LCD update for drawing and interrupts
  io.update();

  // Clock CPU to process interrupts etc.
  cpu.clock();
}

auto GB::insertInterruptOnNextCycle(uint8_t) -> void {
  // TODO
}

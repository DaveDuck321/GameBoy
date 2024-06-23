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

auto GB::readU8(uint16_t addr) const -> Byte {
  return memory_map.read(addr);
}

auto GB::readU16(uint16_t addr) const -> Word {
  return {memory_map.read(addr + 1), memory_map.read(addr)};
}

auto GB::getRegisters() -> CPURegisters& {
  return cpu.getRegisters();
}

auto GB::isSimulationFinished() -> bool {
  return io.isSimulationFinished();
}

auto GB::reset() -> void {
  io.reset();
  memory_map.reset();
  cpu.reset();
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

auto gb::run_standalone(std::unique_ptr<gb::IOFrontend> frontend,
                        std::string_view rom_path) -> void {
  auto gb = std::make_unique<gb::GB>(rom_path, std::move(frontend));
  while (not gb->isSimulationFinished()) {
    gb->clock();
  }
}

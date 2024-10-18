#include "gb.hpp"
#include "cartridge.hpp"
#include "error_handling.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <string_view>
#include <type_traits>

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

auto GB::getCurrentRegisters() -> CPURegisters& {
  return cpu.getCurrentRegisters();
}

auto GB::getDebugRegisters() -> CPURegisters& {
  return cpu.getDebugRegisters();
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

auto gb::run_standalone(gb::GB& gameboy) -> void {
  auto print_reg = []<typename T>(std::string_view reg, T value) {
    if (value.flags.undefined) {
      std::cout << std::format("{}=XX\n", reg);
    } else if (std::is_same_v<T, Byte>) {
      std::cout << std::format("{}={:02x}\n", reg, value.decay());
    } else {
      std::cout << std::format("{}={:04x}\n", reg, value.decay());
    }
  };

  size_t last_debug_trap = 0;
  while (not gameboy.isSimulationFinished()) {
    try {
      gameboy.clock();
    } catch (const DebugTrap&) {
      size_t cycles_since_last = gameboy.io.cycle - last_debug_trap;

      std::cout << "Debug trap!" << std::endl;
      std::cout << "Cycles since last: " << cycles_since_last << std::endl;
      print_reg("a", gameboy.getCurrentRegisters().a);
      print_reg("f", gameboy.getCurrentRegisters()._f);
      print_reg("hl", gameboy.getCurrentRegisters().getU16(Reg16::HL));
      print_reg("b", gameboy.getCurrentRegisters().b);
      print_reg("c", gameboy.getCurrentRegisters().c);
      print_reg("d", gameboy.getCurrentRegisters().d);
      print_reg("e", gameboy.getCurrentRegisters().e);

      last_debug_trap = gameboy.io.cycle;
    }
  }
}

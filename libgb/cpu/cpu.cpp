#include "cpu.hpp"
#include "../io/io.hpp"
#include "../utils/checked_int.hpp"
#include "registers.hpp"

#include <cstdint>
#include <iostream>

using namespace gb;

CPU::CPU(MemoryMap& memory_map, IO& io) : memory_map(&memory_map), io(&io) {}

auto CPU::reset() -> void {
  registers = CPURegisters{};
}

auto CPU::readU8(uint16_t addr) -> Byte {
  // Reads an 8-Bit value from 'addr'
  io->cycle++;  // Under normal circumstances a read takes 1 cycle
  return memory_map->read(addr);
}

auto CPU::readU16(uint16_t addr) -> Word {
  // Reads a 16-Bit LE value from 'addr'
  return {readU8(addr + 1), readU8(addr)};
}

auto CPU::writeU8(uint16_t addr, Byte value) -> void {
  // Writes an 8-Bit value to 'addr'
  io->cycle++;  // Under normal circumstances a write takes 1 cycle
  memory_map->write(addr, value);
}

auto CPU::writeU16(uint16_t addr, Word value) -> void {
  // Writes a 16-Bit LE value to 'addr'
  writeU8(addr, value.lower());
  writeU8(addr + 1, value.upper());
}

auto CPU::advancePC1Byte() -> uint8_t {
  // Returns the 8-Bit value pointed to by the program counter, increments the
  // counter
  return readU8(registers.pc++).decay();
}

auto CPU::advancePC2Bytes() -> uint16_t {
  // Returns the 16-Bit value pointed to by the program counter, increments the
  // counter twice
  Word result = readU16(registers.pc);
  registers.pc += 2;
  return result.decay();
}

auto CPU::handleInterrupts() -> void {
  /*
  Uses CALL to execute an interrupt if it is both triggered and enabled.
  Interrupts (0xFF0F):
      BIT     INTERRUPT           CALL(ADDRESS)
      0       V-Blank             0x40
      1       LCDC Status         0x48
      2       Timer               0x50
      3       Serial              0x58
      4       P10-P13 -> Low      0x60
  */
  uint8_t triggered =
      (memory_map->read(0xFFFF) & memory_map->read(0xFF0F) & 0x1F_B).decay();

  if (triggered != 0) {
    // Interrupt has been triggered, halt should immediately terminate
    registers.halt = false;

    // Dont do anything if interrupts are globally disabled
    if (!registers.IME[0]) {
      return;
    }

    // Disable interrupts, until serviced
    registers.IME.fill(false);
    for (uint8_t bit = 0; bit != 5; bit++) {
      if ((triggered & (1U << bit)) != 0) {
        memory_map->write(0xFF0F, memory_map->read(0xFF0F) ^ Byte(1U << bit));
        CALL_nn(0x40 + 0x08 * bit);
        break;
      }
    }
  }
}

auto CPU::clock() -> void {
  // Check for interrupts (if enabled)
  handleInterrupts();

  // Do nothing if waiting for interrupt
  if (registers.halt) {
    io->cycle++;  // Some time should pass to allow timers to trigger
    return;
  }

  // Advance the program counter
  processNextInstruction();
  registers.IME[0] = registers.IME[1];
  registers.IME[1] = registers.IME[2];
}

auto CPU::getRegisters() -> CPURegisters& {
  return registers;
}

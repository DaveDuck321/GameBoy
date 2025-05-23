#include "cpu.hpp"
#include "../io/io.hpp"
#include "../utils/checked_int.hpp"
#include "registers.hpp"

#include <algorithm>
#include <cstdint>
#include <format>

using namespace gb;

CPU::CPU(MemoryMap& memory_map, IO& io) : memory_map(&memory_map), io(&io) {}

auto CPU::reset() -> void {
  registers = CPURegisters{};
}

auto CPU::readU8(uint16_t addr, bool allow_undef) -> Byte {
  // Reads an 8-Bit value from 'addr'
  io->cycle++;  // Under normal circumstances a read takes 1 cycle
  bool is_high_ram = 0xff80U <= addr && addr <= 0xfffeU;
  if (!is_high_ram && io->isInDMA()) {
    throw_error([&] {
      return DMABusConflict(
          std::format("Read of {:#06x} conflicts with a DMA access", addr));
    });
  }

  Byte result = memory_map->read(addr);
  if (not allow_undef && result.flags.undefined) {
    throw_error([&] {
      return UndefinedDataError(
          std::format("Read of {:#06x} returned undefined memory", addr));
    });
  }
  if (std::ranges::contains(return_address_pointers, addr)) {
    throw_error([&] {
      return ReadingReturnAddressError(std::format(
          "Attempting to read a stack address corresponding to the return "
          "pointer @ {:#06x}",
          addr));
    });
  }
  return result;
}

auto CPU::readU16(uint16_t addr, bool allow_partial_undef) -> Word {
  // Reads a 16-Bit LE value from 'addr'
  Word result = {readU8(addr + 1, allow_partial_undef),
                 readU8(addr, allow_partial_undef)};
  if (allow_partial_undef) {
    if (result.low_undefined && result.high_undefined) {
      throw_error([&] {
        return UndefinedDataError(
            std::format("Read of {:#06x} returned undefined memory", addr));
      });
    }
  } else {
    if (result.flags.undefined) {
      throw_error([&] {
        return UndefinedDataError(
            std::format("Read of {:#06x} returned undefined memory", addr));
      });
    }
  }
  return result;
}

auto CPU::writeU8(uint16_t addr, Byte value, bool allow_undef) -> void {
  // Writes an 8-Bit value to 'addr'
  io->cycle++;  // Under normal circumstances a write takes 1 cycle
  bool is_high_ram = 0xff80U <= addr && addr <= 0xfffeU;
  if (!is_high_ram && io->isInDMA()) {
    throw_error([&] {
      return DMABusConflict(
          std::format("Write of {:#06x} conflicts with a DMA access", addr));
    });
  }
  if (not allow_undef && value.flags.undefined) {
    throw_error([&] {
      return UndefinedDataError("Attempting to write undefined into memory");
    });
  }
  if (std::ranges::contains(return_address_pointers, addr)) {
    throw_error([&] {
      return ClobberedReturnAddressError(std::format(
          "Attempting to clobber a stack address corresponding to the return "
          "pointer @ {:#06x}",
          addr));
    });
  }
  memory_map->write(addr, value);
}

auto CPU::writeU16(uint16_t addr, Word value, bool allow_partial_undef)
    -> void {
  // Writes a 16-Bit LE value to 'addr'
  if (allow_partial_undef) {
    if (value.low_undefined && value.high_undefined) {
      throw_error([&] {
        return UndefinedDataError("Attempting to write undefined into memory");
      });
    }
  } else {
    if (value.flags.undefined) {
      throw_error([&] {
        return UndefinedDataError("Attempting to write undefined into memory");
      });
    }
  }
  writeU8(addr, value.lower(), allow_partial_undef);
  writeU8(addr + 1, value.upper(), allow_partial_undef);
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

  // Instruction was successful, commit registers for easier debugging
  comitted_registers = registers;
}

auto CPU::getCurrentRegisters() -> CPURegisters& {
  return registers;
}

auto CPU::getDebugRegisters() -> CPURegisters& {
  return comitted_registers;
}

#include "io.hpp"

#include "../error_handling.hpp"

#include <cstdint>
#include <format>
#include <utility>

using namespace gb;

constexpr uint_fast16_t IO_OFFSET = 0xFF00;

constexpr uint16_t LCD_STAT = 0xFF41 - IO_OFFSET;

// Interrupts
constexpr uint_fast16_t INTERRUPTS = 0xFF0F - IO_OFFSET;
constexpr uint8_t TIMER_INTERRUPT = 0x04;
constexpr uint8_t SERIAL_INTERRUPT = 0x08;
constexpr uint8_t INPUT_INTERRUPT = 0x10;

// Timers
constexpr uint_fast16_t DIV_TIMER = 0xFF04 - IO_OFFSET;
constexpr uint_fast16_t T_COUNTER = 0xFF05 - IO_OFFSET;
constexpr uint_fast16_t T_MODULO = 0xFF06 - IO_OFFSET;
constexpr uint_fast16_t T_CONTROL = 0xFF07 - IO_OFFSET;

// Sound
constexpr uint_fast16_t NR10_REG = 0xFF10 - IO_OFFSET;
constexpr uint_fast16_t NR52_REG = 0xFF26 - IO_OFFSET;

// Serial
constexpr uint_fast16_t SERIAL_DATA = 0xFF01 - IO_OFFSET;
constexpr uint_fast16_t SERIAL_CTL = 0xFF02 - IO_OFFSET;

auto IO::reset() -> void {
  gpu.reset();
  memory.fill(0);
  inputs = 0xFF;
  lastCycle = 0;
  tCycleCount = 0;
  cycle = 0;
}

auto IO::videoRead(uint16_t addr) const -> uint8_t {
  return gpu.readU8(addr);
}

auto IO::videoWrite(uint16_t addr, uint8_t value) -> void {
  gpu.writeU8(addr, value);
}

auto IO::ioRead(uint16_t addr) -> uint8_t {
  switch (addr) {
    case 0xFF00: {
      // P1/JOYP -- Joypad read
      uint8_t value = 0xFF;
      if ((memory[addr - IO_OFFSET] & 0x10U) == 0) {
        // P14 select and any p14 pressed keys are 0
        value &= 0xE0U | (inputs & 0x0FU);
      }
      if ((memory[addr - IO_OFFSET] & 0x20U) == 0) {
        // P15 select and any p15 pressed keys are 0
        value &= 0xD0U | (uint8_t)(inputs >> 4U);
      }
      return value;
    }
    case 0xFF46:
      // DMA - DMA Transfer and Start Address (W)
      // DMA reads are never allowed
      // There's probably an error somewhere else so throw
      throw IllegalMemoryRead(
          std::format("DMA read (@ {:#06x}) not permitted!", addr));

    // Sound
    case 0xFF10:
      // NR10 - Channel 1 Sweep register (R/W)
      return 0x80U | memory[addr - IO_OFFSET];
    case 0xFF11:
    case 0xFF16:
      // NR11 - Channel 1 Sound length
      return 0x3FU | memory[addr - IO_OFFSET];
    case 0xFF13:
    case 0xFF18:
    case 0xFF1B:
    case 0xFF1D:
    case 0xFF20:
      // NR13, NR23, NR31, NR33, NR41 (W)
      // All write only so ignore read request
      return 0xFF;
    case 0xFF14:
    case 0xFF19:
    case 0xFF23:
    case 0xFF1E:
      // NR14, NR24, NR44, NR34 (R/W)
      // All registers control channel frequency
      // Bit 0-2 and 7 are write only, bits 3-5 are invalid
      return 0xBFU | memory[addr - IO_OFFSET];
    case 0xFF1A:
      // NR30 - Channel 3 Sound on/off (R/W)
      return 0x7FU | memory[addr - IO_OFFSET];
    case 0xFF1C:
      // NR32 - Channel 3 Select output level (R/W)
      return 0x9FU | memory[addr - IO_OFFSET];
    case 0xFF26:
      // NR52 - Sound on/off
      // TODO: real implementation
      return 0x70U | memory[addr - IO_OFFSET];
    case 0xFF27 ... 0xFF2F:
    case 0xFF15:
    case 0xFF1F:
      // Invalid memory, should always return 0xFF
      return 0xFF;

    case 0xFF04:
    case 0xFF05:
      // Timer registers: DIV, TIMA
      // Should only update timers between instructions when accessed
      updateTimers();  // Timers lazy update
                       // Intentional fall through

    default:
      return memory[addr - IO_OFFSET];
  }
}

auto IO::ioWrite(uint16_t addr, uint8_t value) -> void {
  switch (addr) {
    case 0xFF00:
      // P1 Joypad
      memory[addr - IO_OFFSET] = 0xC0U | (value & 0x30U);
      break;
    case 0xFF02:
      // SC -- SIO control (r/w)
      //  Immediately display serial data to output
      frontend->sendSerial(memory[SERIAL_DATA]);
      memory[addr - IO_OFFSET] = value;
      break;
    case 0xFF04:
      // DIV -- Divider Register (cannot write data)
      throw std::runtime_error("DIV write unsupported");
      break;
    case 0xFF05:
      // TIMA -- Timer counter (R/W)
      // Should only update timers between instructions when accessed
      updateTimers();
      memory[addr - IO_OFFSET] = value;
      break;

    // Special LCD registers
    case 0xFF41:
      // LCD Status Register
      memory[LCD_STAT] = 0x80U | (memory[LCD_STAT] & 0x07U) | (value & 0x78U);
      break;
    case 0xFF44:
      // LY -- Scroll Y (r)
      throw std::runtime_error("Cannot write to LY @ 0xFF44");

    // Sound registers
    case 0xFF26:
      // NR52 - Sound on/off
      //  All bits are read only except 7 (Sound on/off)
      if ((value & 0x80U) != 0) {
        powerUpAPU();
      } else {
        powerDownAPU();
      }
      break;
    case 0xFF10 ... 0xFF25:
      // All Sound Control registers
      // Writing to registers should only be possible when APU is powered
      if ((memory[NR52_REG] & 0x80U) == 0) {
        break;
      }
      [[fallthrough]];
    default:
      // Most IO actions don't require immediate action, deal with it later
      memory[addr - IO_OFFSET] = value;
      break;
  }
}

auto IO::reduceTimer(uint16_t threshold) -> void {
  /*
  Reduces the tCycleCount to within 'threshold' by incrementing the timer
  If timer overflows, an interrupt is triggered
  */
  while (tCycleCount >= threshold) {
    tCycleCount -= threshold;

    // Inc counter and detect overflow
    if (((++memory[T_COUNTER]) & 0xFFU) == 0) {
      memory[T_COUNTER] = memory[T_MODULO];
      memory[INTERRUPTS] |= TIMER_INTERRUPT;
    }
  }
}

auto IO::updateTimers() -> void {
  uint64_t dt = cycle - lastCycle;
  lastCycle = cycle;

  // Inc timer by real cycle time
  gpu.updateTimers(dt);
  tCycleCount += dt;
  // Timer increments every 64 cycles
  memory[DIV_TIMER] = (cycle / 64) % 0x100;
  switch (memory[T_CONTROL] & 0x07U) {
    case 0x04:
      // 4096 Hz
      reduceTimer(256);
      break;
    case 0x05:
      // 262144 Hz
      reduceTimer(4);
      break;
    case 0x06:
      // 65536 Hz
      reduceTimer(16);
      break;
    case 0x07:
      // 16384 Hz
      reduceTimer(64);
      break;
    default:
      // Timer is disabled
      break;
  }
}

auto IO::isSimulationFinished() -> bool {
  return frontend->isExitRequested();
}

auto IO::update() -> void {
  updateTimers();

  if (gpu.updateLCD(*frontend)) {
    // Render started, calculate frameskip, get inputs
    const uint8_t keyState = std::to_underlying(frontend->getKeyPressState());
    const uint8_t gbKeyState = ~inputs;

    if ((keyState | gbKeyState) != gbKeyState) {
      // Something was pressed
      memory[INTERRUPTS] |= INPUT_INTERRUPT;
    }
    inputs = ~keyState;
  }
}

auto IO::powerUpAPU() -> void {
  /*
  Powers down the APU and clears all related sound registers.
  This disables all writing until APU is powered up again
  */
  memory[NR52_REG] &= 0x7FU;
  std::fill(memory.begin() + NR10_REG, memory.begin() + NR52_REG, 0);
}

auto IO::powerDownAPU() -> void {
  /*
  Powers up the APU while maintaining register values.
  This enables all writing.
  */
  memory[NR52_REG] |= 0x80U;
}

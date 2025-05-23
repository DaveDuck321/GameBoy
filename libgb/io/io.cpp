#include "io.hpp"
#include "../error_handling.hpp"
#include "io_registers.hpp"

#include <cstdint>
#include <format>
#include <utility>

using namespace gb;
using namespace gb::io_registers;

auto IO::reset() -> void {
  gpu.reset();
  memory.fill(0);
  inputs = 0xFF;
  lastCycle = 0;
  tCycleCount = 0;
  cycle = 0;
}

[[nodiscard]] auto IO::isInDMA() const -> bool {
  return dmaStartTime != 0 && dmaStartTime + 160 > cycle;
}

auto IO::startDMA() -> void {
  dmaStartTime = cycle;
}

auto IO::videoRead(uint16_t addr, bool is_dma) const -> uint8_t {
  return gpu.readU8(addr, is_dma);
}

auto IO::videoWrite(uint16_t addr, uint8_t value, bool is_dma) -> void {
  gpu.writeU8(addr, value, is_dma);
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
      throw_error([&] {
        return IllegalMemoryAddress(
            std::format("DMA read (@ {:#06x}) not permitted!", addr));
      });
      return 0;

    case (IO_OFFSET + FIRST_APU_REGISTER)...(IO_OFFSET + LAST_APU_REGISTER):
      updateTimers();
      return apu.read(addr);
    case 0xFF27 ... 0xFF2F:
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
      // Only send serial data if both the internal clock is selected AND the
      // transfer is enabled (we emulate as-if there is no connected gameboy).
      if ((value & (1U << 7U)) != 0 && (value & 1U) != 0) {
        // Don't bother emulating the serialization delay, immediately output
        // currently loaded data.
        frontend->sendSerial(memory[SERIAL_DATA]);
        memory[addr - IO_OFFSET] = value & ~(1U << 7U);
        memory[SERIAL_DATA] = 0xff;
        memory[INTERRUPTS] |= (1U << 3U);
      } else {
        // Passively commit write to the read/write fields
        memory[SERIAL_DATA] = value | 0b0111'1110U;
      }
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
    case 0xFF40:
      if ((value & 0x80U) == 0 && (memory[addr - IO_OFFSET] & 0x80U) != 0) {
        // We're disable the LCD, this is only allowed in vblank video mode
        if ((memory[LCD_STAT] & 0b11U) != 1U) {
          throw_error([] {
            return LCDDisableViolation(
                "LCD must only be disabled during vblank");
          });
        }
      }
      memory[addr - IO_OFFSET] = value;
      break;

    case 0xFF41:
      // LCD Status Register
      memory[LCD_STAT] = 0x80U | (memory[LCD_STAT] & 0x07U) | (value & 0x78U);
      break;
    case 0xFF44:
      // LY -- Scroll Y (r)
      throw std::runtime_error("Cannot write to LY @ 0xFF44");

    case (IO_OFFSET + FIRST_APU_REGISTER)...(IO_OFFSET + LAST_APU_REGISTER):
      updateTimers();
      apu.write(addr, value);
      break;
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
  apu.clock_to(4 * cycle);

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

  auto audio_samples = apu.get_samples();
  if (auto flushed_count = frontend->try_flush_audio(audio_samples);
      flushed_count.has_value()) {
    apu.flush_samples(flushed_count.value());
  }

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

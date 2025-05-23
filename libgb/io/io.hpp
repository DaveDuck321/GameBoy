#pragma once

#include "apu.hpp"
#include "frontend.hpp"
#include "gpu.hpp"

#include <cstdint>
#include <memory>

namespace gb {

enum class Key : uint8_t {
  NONE = 0x00,
  // P14
  RIGHT = 0x01,
  LEFT = 0x02,
  UP = 0x04,
  DOWN = 0x08,

  // P15
  A = 0x10,
  B = 0x20,
  SELECT = 0x40,
  START = 0x80
};

class IO {
  // IO memory (not including video RAM)
  std::array<uint8_t, 0x80> memory = {};
  GPU gpu;
  APU apu;

  std::unique_ptr<IOFrontend> frontend;

  // Inputs P14 (lower nibble) and P15 (upper nibble)
  uint8_t inputs = 0xFF;

  uint64_t lastCycle = 0;
  uint64_t tCycleCount = 0;
  uint64_t dmaStartTime = 0;

 public:
  uint64_t cycle = 0;

  explicit IO(std::unique_ptr<IOFrontend> frontend)
      : gpu(memory),
        apu(memory, frontend->get_approx_audio_sample_freq()),
        frontend(std::move(frontend)) {}

  auto reset() -> void;

  [[nodiscard]] auto isInDMA() const -> bool;
  auto startDMA() -> void;

  [[nodiscard]] auto videoRead(uint16_t addr, bool is_dma = false) const
      -> uint8_t;
  auto videoWrite(uint16_t addr, uint8_t value, bool is_dma = false) -> void;

  [[nodiscard]] auto ioRead(uint16_t addr) -> uint8_t;
  auto ioWrite(uint16_t addr, uint8_t value) -> void;

  auto isSimulationFinished() -> bool;
  auto update() -> void;

 private:
  auto updateTimers() -> void;
  auto reduceTimer(uint16_t threshold) -> void;
};

}  // namespace gb

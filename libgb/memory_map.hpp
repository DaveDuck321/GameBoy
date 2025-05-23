#pragma once

#include "utils/checked_int.hpp"

#include <array>
#include <cstdint>

namespace gb {

class Cartridge;
class IO;

class MemoryMap {
 private:
  Cartridge* cartridge;
  IO* io;

  // Inline the simple memories
  std::array<Byte, 0x80> stack = {};
  std::array<Byte, 0x2000> workingRam = {};

  void DMA(uint8_t srcUpper);

 public:
  MemoryMap(Cartridge& cartridge, IO& io);

  auto reset() -> void;

  [[nodiscard]] auto read(uint16_t addr, bool is_dma = false) const -> Byte;
  auto write(uint16_t addr, Byte value, bool is_dma = false) -> void;
};

}  // namespace gb

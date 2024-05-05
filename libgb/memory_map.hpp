#pragma once

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
  std::array<uint8_t, 0x80> stack = {};
  std::array<uint8_t, 0x2000> workingRam = {};

  void DMA(uint8_t srcUpper);

 public:
  MemoryMap(Cartridge& cartridge, IO& io);

  [[nodiscard]] auto read(uint16_t addr) const -> uint8_t;
  auto write(uint16_t addr, uint8_t value) -> void;
};

}  // namespace gb

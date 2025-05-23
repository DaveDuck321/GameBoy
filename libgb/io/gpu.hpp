#pragma once

#include "frontend.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace gb {

class GPU {
  /*
  Each tile is 16 bytes.
  Rows are represented by two consecutive bytes.
  Tiles occupy VRAM addresses 8000h-97FFh
  */
  // TODO: use mdspan here
  using Tile = std::array<std::array<uint8_t, 0x02>, 0x08>;
  using Background = std::array<std::array<uint8_t, 0x20>, 0x20>;

  struct SpriteAttribute {
    uint8_t y = 0;
    uint8_t x = 0;
    uint8_t tile = 0;
    uint8_t attribs = 0;
  };

  std::span<uint8_t, 0x80> io_memory;

  std::array<SpriteAttribute, 40> sprites = {};
  std::array<Tile, 0x180> patternTables = {};

  std::array<Background, 2> backgroundMaps = {};

  uint64_t vCycleCount = 0;
  int32_t windowOffsetY = 0;

 public:
  explicit GPU(std::span<uint8_t, 0x80> io_memory);
  auto reset() -> void;

  [[nodiscard]] auto readU8(uint16_t addr, bool is_dma) const -> uint8_t;
  auto writeU8(uint16_t addr, uint8_t value, bool is_dma) -> void;

  auto updateTimers(uint64_t dt) -> void;
  auto updateLCD(IOFrontend&) -> bool;

 private:
  [[nodiscard]] auto byteFromSpriteAttributes(uint16_t addr) const
      -> uint8_t const&;
  [[nodiscard]] auto byteFromPatternTable(uint16_t addr) const
      -> uint8_t const&;
  [[nodiscard]] auto byteFromBackgroundMaps(uint16_t addr) const
      -> uint8_t const&;

  auto renderLine(IOFrontend&) -> void;
  auto setLCDStage(uint8_t stage, bool interrupt) -> bool;
  [[nodiscard]] auto spriteOverridesPixel(int screenX,
                                          int screenY,
                                          uint8_t& color) const -> bool;

  [[nodiscard]] auto pixelFromMap(uint16_t mapX, uint16_t mapY, bool map2) const
      -> uint8_t;
  auto backgroundPixel(int screenX, int screenY, uint8_t& color) const -> void;
  [[nodiscard]] auto windowOverridesPixel(int screenX,
                                          int screenY,
                                          uint8_t& color) const -> bool;
};

}  // namespace gb

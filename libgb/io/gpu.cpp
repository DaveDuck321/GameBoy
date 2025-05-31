#include "gpu.hpp"

#include "../constants.hpp"
#include "../error_handling.hpp"
#include "frontend.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>

using namespace gb;

constexpr uint16_t IO_OFFSET = 0xFF00;

// LCD control
constexpr uint16_t LCDC = 0xFF40 - IO_OFFSET;
constexpr uint16_t LCD_STAT = 0xFF41 - IO_OFFSET;
constexpr uint16_t LCD_LY = 0xFF44 - IO_OFFSET;   // Y-Coordinate
constexpr uint16_t LCD_LYC = 0xFF45 - IO_OFFSET;  // Y-Compare

// Background scroll
constexpr uint16_t BG_SCY = 0xFF42 - IO_OFFSET;
constexpr uint16_t BG_SCX = 0xFF43 - IO_OFFSET;

// Window position
constexpr uint16_t WINDOW_Y = 0xFF4A - IO_OFFSET;
constexpr uint16_t WINDOW_X = 0xFF4B - IO_OFFSET;

// Palettes
constexpr uint16_t BG_Palette = 0xFF47 - IO_OFFSET;
constexpr uint16_t O0_Palette = 0xFF48 - IO_OFFSET;
constexpr uint16_t O1_Palette = 0xFF49 - IO_OFFSET;

// Interrupts
constexpr uint16_t INTERRUPTS = 0xFF0F - IO_OFFSET;
constexpr uint8_t VSYNC_INTERRUPT = 0x01;
constexpr uint8_t STAT_INTERRUPT = 0x02;

GPU::GPU(std::span<uint8_t, 0x80> io_memory) : io_memory(io_memory) {
  reset();
}

auto GPU::reset() -> void {
  sprites = {};
  patternTables = {};
  backgroundMaps = {};
  vCycleCount = 0;
  windowOffsetY = 0;
}

[[nodiscard]] auto GPU::readU8(uint16_t addr, bool is_dma) const -> uint8_t {
  auto mode = io_memory[LCD_STAT] & 0b11U;
  switch (addr) {
    case 0x8000 ... 0x97FF:
      // Tile data 1
      if (!is_dma && mode == 3U) {
        throw_error([] {
          return PPUViolation("Reading from tile data during pixel blitz");
        });
      }
      return byteFromPatternTable(addr);
    case 0x9800 ... 0x9FFF:
      // Background maps
      if (!is_dma && mode == 3U) {
        throw_error([] {
          return PPUViolation(
              "Reading from background maps during pixel blitz");
        });
      }
      return byteFromBackgroundMaps(addr);
    case 0xFE00 ... 0xFE9F:
      // Sprite attributes
      if (!is_dma && (mode == 2U || mode == 3U)) {
        throw_error([] {
          return PPUViolation(
              "Reading from sprit attribute data during pixel blitz/ OAM scan");
        });
      }
      return byteFromSpriteAttributes(addr);
    default:
      throw std::range_error("Bad vram address read");
  }
}

auto GPU::writeU8(uint16_t addr, uint8_t value, bool is_dma) -> void {
  auto mode = io_memory[LCD_STAT] & 0b11U;

  // Never allowed to write undefined to the GPU memory
  switch (addr) {
    case 0x8000 ... 0x97FF:
      // Tile data 1
      if (!is_dma && mode == 3U) {
        throw_error([] {
          return PPUViolation("Writing to tile data during pixel blitz");
        });
      }
      const_cast<uint8_t&>(byteFromPatternTable(addr)) = value;
      break;
    case 0x9800 ... 0x9FFF:
      // Background maps
      if (!is_dma && mode == 3U) {
        throw_error([] {
          return PPUViolation("Writing to background maps during pixel blitz");
        });
      }
      const_cast<uint8_t&>(byteFromBackgroundMaps(addr)) = value;
      break;
    case 0xFE00 ... 0xFE9F:
      // Sprite attributes
      if (!is_dma && (mode == 2U || mode == 3U)) {
        throw_error([] {
          return PPUViolation(
              "Writing to sprit attribute data during pixel blitz/ OAM scan");
        });
      }
      const_cast<uint8_t&>(byteFromSpriteAttributes(addr)) = value;
      break;
    default:
      throw std::range_error("Bad vram address write");
  }
}

auto GPU::byteFromSpriteAttributes(uint16_t addr) const -> uint8_t const& {
  uint16_t base_offset = addr - 0xFE00U;
  uint16_t attribute_index = base_offset / 4;
  switch (base_offset % 4) {
    case 0:
      return sprites.at(attribute_index).y;
    case 1:
      return sprites.at(attribute_index).x;
    case 2:
      return sprites.at(attribute_index).tile;
    case 3:
      return sprites.at(attribute_index).attribs;
    default:
      assert(!"Unreachable");
  }
}

auto GPU::byteFromPatternTable(uint16_t addr) const -> uint8_t const& {
  uint16_t base_offset = addr - 0x8000U;
  uint16_t tile_base = base_offset / 0x10U;
  uint16_t offset_in_tile = base_offset % 0x10U;
  uint16_t row_in_tile = offset_in_tile / 2;
  uint16_t byte_in_row = offset_in_tile % 2;
  return patternTables.at(tile_base).at(row_in_tile).at(byte_in_row);
}

[[nodiscard]] auto GPU::byteFromBackgroundMaps(uint16_t addr) const
    -> uint8_t const& {
  uint16_t base_offset = addr - 0x9800U;
  uint16_t map_index = base_offset / (0x20 * 0x20);
  uint16_t map_offset = base_offset % (0x20 * 0x20);
  return backgroundMaps.at(map_index)
      .at(map_offset / 0x20U)
      .at(map_offset % 0x20U);
}

auto GPU::renderLine(IOFrontend& frontend) -> void {
  /*
  Draws the current line (index 0xFF44) onto the display
  This draws: background, window, sprites
  */
  int screenY = io_memory[LCD_LY];
  if (io_memory[WINDOW_X] <= 166 || (io_memory[LCDC] & 0x20U) != 0) {
    windowOffsetY++;
  }
  for (int screenX = 0; screenX < SCREEN_WIDTH; screenX++) {
    uint8_t pixelColor = 0;
    if ((io_memory[LCDC] & 0x02U) != 0 &&
        spriteOverridesPixel(screenX, screenY, pixelColor)) {
      frontend.addPixel(pixelColor, screenX, screenY);
      continue;
    }
    if ((io_memory[LCDC] & 0x20U) != 0 &&
        windowOverridesPixel(screenX, screenY, pixelColor)) {
      frontend.addPixel(pixelColor, screenX, screenY);
      continue;
    }
    if ((io_memory[LCDC] & 0x01U) != 0) {
      backgroundPixel(screenX, screenY, pixelColor);
    }
    frontend.addPixel(pixelColor, screenX, screenY);
  }
}

auto GPU::setLCDStage(uint8_t stage, bool interrupt) -> bool {
  /*
  Sets the first 2 bits of the LCD_STAT register to the selected stage.
  Returns true if a new stage is entered
  If enabled and a new stage is entered, trigger the interrupt flag.
  */
  if ((io_memory[LCD_STAT] & 0x03U) != stage) {
    // Trigger interrupt if stage changed and interrupt enabled
    if (interrupt) {
      io_memory[INTERRUPTS] |= STAT_INTERRUPT;
    }

    io_memory[LCD_STAT] = (io_memory[LCD_STAT] & 0xFCU) | stage;
    return true;
  }
  return false;
}

auto GPU::updateTimers(uint64_t dt) -> void {
  vCycleCount += 4 * dt;
}

auto GPU::updateLCD(IOFrontend& frontend) -> bool {
  // Timings from http://bgb.bircd.org/pandocs.htm#videodisplay
  // y-scan should increment throughout the entire draw process
  if ((io_memory[LCDC] & 0x80U) == 0) {
    vCycleCount = 0;
    io_memory[LCD_LY] = 0;
    io_memory[LCD_STAT] = (io_memory[LCD_STAT] & 0xFCU);
    return false;
  }
  io_memory[LCD_LY] = vCycleCount / 456;

  switch (vCycleCount) {
    case 0 ... 65663:
      // Drawing to screen
      switch (vCycleCount % 456) {
        case 0 ... 77:
          // Mode 2: (don't need to emulate OAM)
          // Needs to trigger interrupt if enabled
          if (setLCDStage(0x02U, io_memory[LCD_STAT] & 0x20U)) {
            // Set compare register and trigger interrupts if needed
            io_memory[LCD_STAT] =
                (io_memory[LCD_STAT] & 0xFBU) |
                ((io_memory[LCD_LY] == io_memory[LCD_LYC]) << 2U);
            if ((io_memory[LCD_STAT] & 0x40U) && (io_memory[LCD_STAT] & 0x04U))
              io_memory[INTERRUPTS] |= STAT_INTERRUPT;
          }
          break;
        case 78 ... 246:
          // Mode 3: (don't need to emulate OAM)
          io_memory[LCD_STAT] = (io_memory[LCD_STAT] & 0xFCU) | 0x03U;
          break;
        default:
          // Mode 0: scan line needs to be drawn
          if (setLCDStage(0x00, io_memory[LCD_STAT] & 0x08U)) {
            // Only attempt draw once per line
            // Only draw when frame requested
            if (frontend.isFrameScheduled()) {
              renderLine(frontend);
            }
          }
          break;
      }
      break;
    case 65664 ... 70223:
      // Mode 1: VBlank period
      if (setLCDStage(0x01U, io_memory[LCD_STAT] & 0x10U)) {
        // Always trigger vsync interrupt
        io_memory[INTERRUPTS] |= VSYNC_INTERRUPT;
      }
      break;
    default:
      // VBlank finished... flush screen
      frontend.commitRender();
      // Reset registers
      io_memory[LCD_LY] = 0;
      vCycleCount = 0;
      windowOffsetY = 0;
      return true;
  }
  return false;
}

auto GPU::spriteOverridesPixel(int screenX, int screenY, uint8_t& color) const
    -> bool {
  /*
  Uses the sprite attributes and pattern data to set &color
  If this color is definitely in the forground, returns true.
  If no color can be found or the color is transparent, color is not modified
  */
  // Set to 8 of 16 height mode
  uint8_t height = 8 + ((io_memory[LCDC] & 0x04U) << 1U);

  // Sprites are enabled, draw them
  size_t sprites_on_scanline = 0;
  for (const SpriteAttribute& attribs : sprites) {
    if ((attribs.y > screenY + 16) || (attribs.y + height <= screenY + 16)) {
      continue;
    }

    sprites_on_scanline += 1;
    if (sprites_on_scanline > 10) {
      // The PPU selects up to 10 objects sequentially from OAM
      continue;
    }

    // Check if sprite contains x coord
    if ((attribs.x > screenX + 8) || (attribs.x <= screenX)) {
      continue;
    }

    // Pixel is definitely in current sprite
    // Get relative tile coord
    uint8_t tileX = (8 + screenX) - attribs.x;
    uint8_t tileY = (16 + screenY) - attribs.y;

    // Mirror patterns if attrib is set
    if ((attribs.attribs & 0x20U) != 0) {
      tileX = 7 - tileX;
    }
    if ((attribs.attribs & 0x40U) != 0) {
      tileY = height - tileY;
    }

    // Gets the pattern table index of the current tile
    uint8_t tileIndex = attribs.tile;
    if ((io_memory[LCDC] & 0x04U) != 0) {
      // 16px height -- ignore lower bit
      tileIndex = (attribs.tile & 0xFEU) + (uint8_t)(tileY > 7U);
    }

    // Extract color from tile and color palette
    const Tile& tile = patternTables[tileIndex];
    bool upper = ((tile[tileY % 8][1] >> (7 - tileX)) & 1);
    bool lower = ((tile[tileY % 8][0] >> (7 - tileX)) & 1);
    uint8_t colorIndex = (upper << 1) | lower;

    if (colorIndex == 0) {
      continue;  // Sprite at this location is transparent
    }

    // Select the color palette
    uint8_t colorPalette =
        io_memory[O0_Palette + ((attribs.attribs & 0x10U) >> 4U)];

    // Sets color reference and returns
    color = (colorPalette >> (2 * colorIndex)) & 0x03U;

    // First sprite has draw priority so return immediately
    return !(attribs.attribs &
             0x80U);  // Attrib 7 determines forground priority
  }
  return false;
}

auto GPU::pixelFromMap(uint16_t mapX, uint16_t mapY, bool map2) const
    -> uint8_t {
  /*
  Returns the pixel located at the corresponding map coordinates
  When map2 == False: backgroundMap1 is used for tile resolution
  When map2 == True: backgroundMap2 is used for tile resolution
  */

  // Read tile index from correct background map
  uint16_t tileIndex = backgroundMaps[0][(mapY / 8) % 0x20][(mapX / 8) % 0x20];
  if (map2) {
    tileIndex = backgroundMaps[1][(mapY / 8) % 0x20][(mapX / 8) % 0x20];
  }

  // Correct index using the signed lookup table if requested
  if ((io_memory[LCDC] & 0x10U) == 0) {
    tileIndex = 0x100 + (int8_t)(tileIndex & 0xFFU);
  }

  const Tile& tile = patternTables[tileIndex];
  uint8_t upper = (tile[mapY % 8][1] >> (7 - mapX % 8)) & 1;
  uint8_t lower = (tile[mapY % 8][0] >> (7 - mapX % 8)) & 1;
  uint8_t colorIndex = (upper << 1U) + lower;

  // Gets the true background color
  return (io_memory[BG_Palette] >> (2 * colorIndex)) & 0x03U;
}

void GPU::backgroundPixel(int screenX, int screenY, uint8_t& color) const {
  /*
  Uses the background map and pattern data to set &color
  If background color is zero, color is not modified
  */
  int bgX = screenX + io_memory[BG_SCX];
  int bgY = screenY + io_memory[BG_SCY];

  uint8_t bgColor = pixelFromMap(bgX, bgY, io_memory[LCDC] & 0x08U);
  if (bgColor != 0) {
    color = bgColor;
  }
}

auto GPU::windowOverridesPixel(int screenX, int screenY, uint8_t& color) const
    -> bool {
  (void)screenY;
  /*
  Uses the window map and pattern data to set &color
  Window is always drawn on top of background
  */
  int windowX = screenX - io_memory[WINDOW_X] + 7;
  int windowY = windowOffsetY - io_memory[WINDOW_Y];

  if (io_memory[WINDOW_X] > 166) {
    return false;
  }
  if (windowX >= 0 && windowX < 160 && windowY >= 0 && windowY < 144) {
    color = pixelFromMap(windowX, windowY, io_memory[LCDC] & 0x40U);
    return true;
  }
  return false;
}

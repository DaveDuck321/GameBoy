#include "display.hpp"

uint8_t Display::read(uint16_t addr) const
{
    switch(addr)
    {
        case 0x8000 ... 0x97FF:
            // Tile data 1
            // Flatten 3D array and write to it like the gameboy would
            // (Might cause errors on some compilers -- TODO: check)
            static_assert(sizeof(patternTables) == 0x1800);
            return (reinterpret_cast<const uint8_t *>(&patternTables))[addr-0x8000];
        case 0x9800 ... 0x9BFF:
            //Background map 1
            static_assert(sizeof(backgroundMap1) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap1))[addr-0x9800];
        case 0x9C00 ... 0x9FFF:
            //Background map 1
            static_assert(sizeof(backgroundMap2) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap2))[addr-0x9C00];
        default:
            throw std::range_error("Bad vram address read");
    }
}

void Display::write(uint16_t addr, uint8_t value)
{
    switch(addr)
    {
        case 0x8000 ... 0x97FF:
            // Tile data 1
            static_assert(sizeof(patternTables) == 0x1800);
            (reinterpret_cast<uint8_t *>(&patternTables))[addr-0x8000] = value;
            break;
        case 0x9800 ... 0x9BFF:
            //Background map 1
            static_assert(sizeof(backgroundMap1) == 0x400);
            (reinterpret_cast<uint8_t *>(&backgroundMap1))[addr-0x9800] = value;
            break;
        case 0x9C00 ... 0x9FFF:
            //Background map 2
            static_assert(sizeof(backgroundMap2) == 0x400);
            (reinterpret_cast<uint8_t *>(&backgroundMap2))[addr-0x9C00] = value;
            break;
        default:
            throw std::range_error("Bad vram address write");
    }
}

void Display::draw() const
{
    // Allow implementation to initialize
    clearScreen();

    // Draw background 1
    for(uint8_t x=0; x<0x20; x++)
    {
        for(uint8_t y=0; y<0x20; y++)
        {
            uint8_t tileIndex = backgroundMap1[y][x];
            drawTile(patternTables[tileIndex], x*8, y*8);
        }
    }

    // For if implementation needs to finish rendering
    finishRender();
}
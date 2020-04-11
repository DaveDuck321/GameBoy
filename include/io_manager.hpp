#ifndef io_h
#define io_h

#include <memory>


typedef std::array<std::array<uint8_t, 0x02>, 0x08> Tile;
typedef std::array<uint8_t, 4> SpriteAttribute;

//General GameBoy info

const uint8_t SCREEN_WIDTH = 160;
const uint8_t SCREEN_HEIGHT = 144;

const uint_fast16_t IO_OFFSET = 0xFF00;

//Interrupts
const uint_fast16_t INTERRUPTS = 0xFF0F - IO_OFFSET;

//LCD control
const uint_fast16_t LCDC = 0xFF40       - IO_OFFSET;
const uint_fast16_t LCD_STAT = 0xFF41   - IO_OFFSET;
const uint_fast16_t LCD_LY = 0xFF44     - IO_OFFSET; //Y-Coordinate
const uint_fast16_t LCD_LYC = 0xFF45    - IO_OFFSET; //Y-Compare

//Background scroll
const uint_fast16_t GB_SCY = 0xFF42     - IO_OFFSET;
const uint_fast16_t GB_SCX = 0xFF43     - IO_OFFSET;

//Window position
const uint_fast16_t WINDOW_Y = 0xFF4A   - IO_OFFSET;
const uint_fast16_t WINDOW_X = 0xFF4B   - IO_OFFSET;

//Palettes
const uint_fast16_t BG_Palette = 0xFF47 - IO_OFFSET;
const uint_fast16_t O0_Palette = 0xFF48 - IO_OFFSET;
const uint_fast16_t O1_Palette = 0xFF49 - IO_OFFSET;

//Interrupts
const uint8_t VSYNC_INTERRUPT = 0x01;
const uint8_t STAT_INTERRUPT = 0x02;

class IO_Manager
{
    private:
    // Cycle counter (for drawing)
    uint64_t startCycle = 0;
    uint64_t cycleCounter = 0;
    // IO memory (not including video RAM)
    std::array<uint8_t, 0x4C> memory;

    protected:
    // Color pallets map a 2 bit pixel to a color

    /*
    Each tile is 16 bytes.
    Rows are represented by two consecutive bytes.
    Tiles occupy VRAM addresses 8000h-97FFh
    */
    std::array<SpriteAttribute, 40> sprites;
    std::array<Tile, 0x180> patternTables;

    std::array<std::array<uint8_t, 0x20>, 0x20> backgroundMap1;
    std::array<std::array<uint8_t, 0x20>, 0x20> backgroundMap2;

    public:

    // Common IO stuff
        IO_Manager();
        uint8_t videoRead(uint16_t addr) const;
        void videoWrite(uint16_t addr, uint8_t value);

        uint8_t ioRead(uint16_t addr) const;
        void ioWrite(uint16_t addr, uint8_t value);

    // Display stuff
        void updateLCD(uint64_t cycle);
        bool spriteOverridesPixel(int screenX, int screenY,  uint8_t &color) const;
        void backgroundPixel(int screenX, int screenY,  uint8_t &color) const;

        void drawLine() const;

        virtual void finishRender() const {};
        // TODO: Add color palette parameter
        virtual void drawPixel(int color, int screenX, int screenY) const = 0;
};

#endif
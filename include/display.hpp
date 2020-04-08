#ifndef display_h
#define display_h

#include <array>

typedef std::array<std::array<uint8_t, 0x02>, 0x08> Tile;

class Display
{
    protected:
    // Color pallets map a 2 bit pixel to a color

    /*
    Each tile is 16 bytes.
    Rows are represented by two consecutive bytes.
    Tiles occupy VRAM addresses 8000h-97FFh
    */
    std::array<Tile, 0x180> patternTables;

    std::array<std::array<uint8_t, 0x20>, 0x20> backgroundMap1;
    std::array<std::array<uint8_t, 0x20>, 0x20> backgroundMap2;

    public:
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);

    void draw() const;

    virtual void clearScreen() const = 0;
    virtual void finishRender() const {};
    // TODO: Add color pallet parameter
    virtual void drawTile(Tile tile, int screenX, int screenY) const = 0;
};

#endif
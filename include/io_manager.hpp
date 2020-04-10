#ifndef io_h
#define io_h

#include <memory>

typedef std::array<std::array<uint8_t, 0x02>, 0x08> Tile;

class IO_Manager
{
    private:
    // IO memory (not including video RAM)
    std::array<uint8_t, 0x4C> memory;

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

    // Common IO stuff
        IO_Manager();
        uint8_t vramRead(uint16_t addr) const;
        void vramWrite(uint16_t addr, uint8_t value);

        uint8_t ioRead(uint16_t addr) const;
        void ioWrite(uint16_t addr, uint8_t value);

    // Display stuff
        void draw() const;

        virtual void clearScreen() const = 0;
        virtual void finishRender() const {};
        // TODO: Add color palette parameter
        virtual void drawTile(Tile tile, int screenX, int screenY) const = 0;
};

#endif
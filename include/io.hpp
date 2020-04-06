#ifndef io_h
#define io_h

#include <memory>

const std::size_t OFFSET_ADDR = 0xFF00;

const std::size_t LY_ADDR = 0xFF44 - OFFSET_ADDR;
const std::size_t BGP_ADDR = 0xFF47 - OFFSET_ADDR;
const std::size_t OBP0_ADDR = 0xFF48 - OFFSET_ADDR;
const std::size_t OBP1_ADDR = 0xFF49 - OFFSET_ADDR;
const std::size_t WY_ADDR = 0xFF4A - OFFSET_ADDR;
const std::size_t WX_ADDR = 0xFF4B - OFFSET_ADDR;

class IO
{
    private:
    std::array<uint8_t, 0x4C> memory;

    public:
    void write(uint16_t addr, uint8_t value);
    uint8_t read(uint16_t addr);
};

#endif
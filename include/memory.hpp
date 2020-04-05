#ifndef memory_h
#define memory_h

#include "cartridge.hpp"
#include <array>

class Memory
{
    private:
    Cartridge &cartridge;

    std::array<uint8_t, 0x80> stack;
    std::array<uint8_t, 0x80> IO;
    std::array<uint8_t, 0x2000> workingRam;

    public:
    Memory(Cartridge &cartridge);
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);
};

#endif
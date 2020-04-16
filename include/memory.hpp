#ifndef memory_h
#define memory_h

#include "cartridge.hpp"
#include "io_manager.hpp"

#include <array>

class Memory
{
    private:
    Cartridge &cartridge;
    IO_Manager &io;

    std::array<uint8_t, 0x80> stack;
    std::array<uint8_t, 0x2000> workingRam;

    void DMA(uint8_t uppeerAddr);

    public:
    Memory(Cartridge &cartridge, IO_Manager &io);
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);
};

#endif
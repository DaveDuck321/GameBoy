#ifndef controller_h
#define controller_h

#include <memory>
#include <vector>

class Controller
{
    public:
    std::vector<uint8_t> rom;

    Controller(std::vector<uint8_t>&& rom): rom(rom)
    {
        
    }

    virtual uint8_t read(uint8_t addr) const
    {
        if(addr < 0x4000) return rom[addr];
        throw std::range_error("Could not read memory location in ROM");
    }

    virtual void write(uint16_t addr, uint8_t value)
    {
        throw std::runtime_error("Cannot write to ROM only card");
    }
};

#endif
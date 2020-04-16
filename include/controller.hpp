#ifndef controller_h
#define controller_h

#include <memory>
#include <vector>
#include <iostream>

class Controller
{
    protected:
    const std::vector<uint8_t> &rom;

    public:
    Controller(const std::vector<uint8_t>& rom): rom(rom) {}

    virtual uint8_t read(uint16_t addr) const
    {
        return rom[addr];
    }

    virtual void write(uint16_t addr, uint8_t value)
    {
        //ROM should just ignore write errors
        //Some games write to the controller even if there's just ROM
        //Print message for debugging anyway
        std::cout << "ROM write requested! Addr: " << addr << " Value: " << (int)value << std::endl;
    }
};

#endif
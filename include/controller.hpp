#ifndef controller_h
#define controller_h

#include <memory>
#include <vector>

class Controller
{
    protected:
    std::vector<uint8_t> rom;

    public:
    Controller(std::vector<uint8_t>&& rom): rom(rom) {}

    virtual uint8_t read(uint16_t addr) const
    {
        return rom[addr];
    }

    virtual void write(uint16_t addr, uint8_t value)
    {
        throw std::runtime_error("Cannot write to ROM only card");
    }
};

#endif
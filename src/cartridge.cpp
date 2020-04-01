#include "cartridge.hpp"
#include <memory>
#include <fstream>

Cartridge Cartridge::loadRom(const std::string& name)
{
    std::ifstream input(name, std::ios::binary);
    Cartridge c(std::istreambuf_iterator<char>(input), {});
    return c;
};

uint8_t Cartridge::read(uint16_t location)
{
    if(location < 0x4000) return rom[location];
    throw std::runtime_error("Could not read memory location in ROM");
}
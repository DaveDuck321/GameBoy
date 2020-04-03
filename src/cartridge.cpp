#include "cartridge.hpp"
#include <memory>
#include <fstream>

Cartridge Cartridge::loadRom(const std::string& name)
{
    std::ifstream input(name, std::ios::binary);
    Cartridge c(std::istreambuf_iterator<char>(input), {});
    return c;
};

void Cartridge::populateMetadata()
{
    /*
    Populates useful cartridge information from the ROM.

    Magic numbers from here:
    http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
    */

    //GB Color bit
    gbType = read(0x143);

    //Name in upper ASCII
    gameName = std::string(rom.begin()+0x134, rom.begin()+0x142);

    //Describes the cartridge technology used
    //This might be misrepresented by the game -- could cause errors later
    cartridgeType = read(0x147);

    //ROM size and type -- enum type
    romSize = read(0x148);

    //RAM size and type -- enum type
    ramSize = read(0x149);
}

uint8_t Cartridge::read(uint16_t addr) const
{
    if(addr < 0x4000) return rom[addr];
    throw std::range_error("Could not read memory location in ROM");
}

void Cartridge::write(uint16_t addr, uint8_t value)
{
    throw std::runtime_error("Cannot write to ROM only card");
}

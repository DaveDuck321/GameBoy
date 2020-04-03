#include "cartridge.hpp"

#include <memory>
#include <fstream>

Cartridge Cartridge::loadRom(const std::string& name)
{
    std::ifstream input(name, std::ios::binary);
    
    return Cartridge(
        std::vector<uint8_t>(
            std::istreambuf_iterator<char>(input),
            {}
        )
    );
};

Cartridge::Cartridge(std::vector<uint8_t>&& rom)
{
    //populateMetadata(rom);
    controller = std::make_unique<Controller>(std::move(rom));
}

void Cartridge::populateMetadata(const std::vector<uint8_t>& rom)
{
    /*
    Populates useful cartridge information from the ROM.

    Magic numbers from here:
    http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
    */

    //GB Color bit
    gbType = rom[0x143];

    //Name in upper ASCII
    gameName = std::string(rom.begin()+0x134, rom.begin()+0x142);

    //Describes the cartridge technology used
    //This might be misrepresented by the game -- could cause errors later
    cartridgeType = rom[0x147];

    //ROM size and type -- enum type
    romSize = rom[0x148];

    //RAM size and type -- enum type
    ramSize = rom[0x149];
}

uint8_t Cartridge::read(uint16_t addr) const
{
    controller->read(addr);
}

void Cartridge::write(uint16_t addr, uint8_t value)
{
    controller->write(addr, value);
}

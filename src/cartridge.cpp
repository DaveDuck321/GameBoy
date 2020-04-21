#include "cartridge.hpp"
#include "controller.hpp"
#include "controllers/mbc1.hpp"

#include <fstream>

Cartridge Cartridge::loadRom(const std::string& name)
{
    std::ifstream input(name, std::ios::binary);
    if(!input)  throw std::runtime_error("Couldn't read ROM");

    return Cartridge(
        std::vector<uint8_t>(
            std::istreambuf_iterator<char>(input),
            {}
        )
    );
}

Cartridge::Cartridge(std::vector<uint8_t>&& rom): rom(rom)
{
    populateMetadata(this->rom);
    switch(cartridgeType)
    {
        case 0: // ROM only
            controller = std::make_unique<Controller>(
                this->rom
            );
            break;
        case 1: case 2: case 3: // MBC1 Controller
            controller = std::make_unique<MBC1>(
                this->rom
            );
            break;
        default:
            throw std::runtime_error("Cartridge controller not implemented");
    }
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
    return controller->read(addr);
}

void Cartridge::write(uint16_t addr, uint8_t value)
{
    controller->write(addr, value);
}

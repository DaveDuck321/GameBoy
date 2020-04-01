#include "gb.hpp"

#include <iostream>

GB::GB(Cartridge &cartridge): cartridge(cartridge), registers(this)
{

}

GB::~GB()
{

}

uint8_t GB::pcPopU8()
{
    return readU8(registers.pc++);
}

uint16_t GB::pcPopU16()
{
    uint16_t result = readU16(registers.pc);
    registers.pc += 2;
    return result;
}

void GB::writeU8(uint16_t addr, uint8_t value)
{
    throw("Not implemented");
}

void GB::writeU16(uint16_t addr, uint16_t value)
{
    throw("Not implemented");
}

uint8_t GB::readU8(uint16_t addr) const
{
    switch (addr)
    {
    case 0x0000 ... 0x7FFF:
        // Rom
        cartridge.read(addr);
        break;
    case 0x8000 ... 0x9FFF:
        // Video Ram
        break;
    case 0xA000 ... 0xBFFF:
        // External ram
        break;
    case 0xC000 ... 0xDFFF:
        // Work ram 2
        break;
    case 0xE000 ... 0xFDFF:
        //0xC000 - 0xDDFF echo ram
        break;
    case 0xFE00 ... 0xFE9F:
        // Sprite Attribute Table
        break;
    case 0xFEA0 ... 0xFEFF:
        // Not Usable
        break;
    case 0xFF00 ... 0xFF7F:
        // IO ports
        break;
    case 0xFF80 ... 0xFFFE:
        // High ram
        break;
    case 0xFFFF:
        // Interrupt Enable Register
        break;
    default:
        throw("Impossible memory address");
    }
}

uint16_t GB::readU16(uint16_t addr) const
{
    throw("Not implemented");
}

int main()
{
    Cartridge card = Cartridge::loadRom("roms/Tetris.gb");
    GB gb(card);
    std::cout << (int)gb.readU8(0x3ffe) << std::endl;
    

    return EXIT_SUCCESS;
}
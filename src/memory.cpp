#include "memory.hpp"

#include <iostream>

Memory::Memory(Cartridge &cartridge, IO &io, Display &display):
    cartridge(cartridge), display(display), io(io)
{
    stack.fill(0xF4); //Debug checker
    workingRam.fill(0xF4);
    //Power up sequence (from http://bgb.bircd.org/pandocs.htm)
    write(0xFF05, 0x00); //TIMA
    write(0xFF06, 0x00); //TMA
    write(0xFF07, 0x00); //TAC
    write(0xFF10, 0x80); //NR10
    write(0xFF11, 0xBF); //NR11
    write(0xFF12, 0xF3); //NR12
    write(0xFF14, 0xBF); //NR14
    write(0xFF16, 0x3F); //NR21
    write(0xFF17, 0x00); //NR22
    write(0xFF19, 0xBF); //NR24
    write(0xFF1A, 0x7F); //NR30
    write(0xFF1B, 0xFF); //NR31
    write(0xFF1C, 0x9F); //NR32
    write(0xFF1E, 0xBF); //NR33
    write(0xFF20, 0xFF); //NR41
    write(0xFF21, 0x00); //NR42
    write(0xFF22, 0x00); //NR43
    write(0xFF23, 0xBF); //NR30
    write(0xFF24, 0x77); //NR50
    write(0xFF25, 0xF3); //NR51
    //writeU8(0xFF26, 0xF1-GB, $F0-SGB //NR52
    write(0xFF40, 0x91); //LCDC
    write(0xFF42, 0x00); //SCY
    write(0xFF43, 0x00); //SCX
    write(0xFF45, 0x00); //LYC
    write(0xFF47, 0xFC); //BGP
    write(0xFF48, 0xFF); //OBP0
    write(0xFF49, 0xFF); //OBP1
    write(0xFF4A, 0x00); //WY
    write(0xFF4B, 0x00); //WX
    write(0xFFFF, 0x00); //IE

    write(0xFF0F, 0x00); //Interrupts
}

uint8_t Memory::read(uint16_t addr) const
{
    switch (addr)
    {
    case 0x0000 ... 0x7FFF:
        // Rom
        return cartridge.read(addr);
    case 0x8000 ... 0x9FFF:
        // Video Ram
        return display.read(addr);
    case 0xA000 ... 0xBFFF:
        // External ram
        return cartridge.read(addr);
    case 0xC000 ... 0xDFFF:
        // Work ram 2
        return workingRam[addr-0xC000];
    case 0xE000 ... 0xFDFF:
        //0xC000 - 0xDDFF echo ram
        return workingRam[addr-0xE000];
    case 0xFE00 ... 0xFE9F:
        // Sprite Attribute Table
        throw std::runtime_error("Sprites not implemented");
    case 0xFEA0 ... 0xFEFF:
        // Not Usable
        throw std::runtime_error("Bad ram address");
    case 0xFF00 ... 0xFF7F:
        //IO ports
        return io.read(addr);
    case 0xFF80 ... 0xFFFE:
        // High ram
        return stack[addr - 0xFF80];
    case 0xFFFF:
        // Interrupts enabled Register
        return stack[0x7F];
    default:
        std::cout << "Impossible memory address: " << addr << std::endl;
        throw std::range_error("Impossible memory address");
    }
}

void Memory::write(uint16_t addr, uint8_t value)
{
    switch (addr)
    {
    case 0x0000 ... 0x7FFF:
        // Rom
        cartridge.write(addr, value);
        break;
    case 0x8000 ... 0x9FFF:
        // Video Ram
        display.write(addr, value);
        break;
    case 0xA000 ... 0xBFFF:
        // External ram
        cartridge.write(addr, value);
        break;
    case 0xC000 ... 0xDFFF:
        // Work ram 2
        workingRam[addr-0xC000] = value;
        break;
    case 0xE000 ... 0xFDFF:
        //0xC000 - 0xDDFF echo ram
        workingRam[addr-0xE000] = value;
        break;
    case 0xFE00 ... 0xFE9F:
        // Sprite Attribute Table
        throw std::runtime_error("Sprites not implemented");
        break;
    case 0xFEA0 ... 0xFEFF:
        // Not Usable
        throw std::runtime_error("Bad ram address");
        break;
    case 0xFF00 ... 0xFF7F:
        //IO ports
        io.write(addr, value);
        break;
    case 0xFF80 ... 0xFFFE:
        // High ram
        stack[addr - 0xFF80] = value;
        break;
    case 0xFFFF:
        // Interrupts enabled Register
        stack[0x7F] = value;
        break;
    default:
        std::cout << "Impossible memory address: " << addr << std::endl;
        throw std::runtime_error("Impossible memory address");
    }
}

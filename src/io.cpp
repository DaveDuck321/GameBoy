#include "io.hpp"

#include <iostream>

IO::IO()
{
    memory.fill(0xF4); //For debuging
}

void IO::write(uint16_t addr, uint8_t value)
{
    uint8_t offsetAddr = addr-OFFSET_ADDR;
    switch (addr)
    {
    case 0xFF00:
        //P1 -- joypad info (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF01:
        //SB -- Serial transfer data (r/w)
        //std::cout << "SB: "<< value <<std::endl;
        std::cout<<value;
        memory[offsetAddr] = value;
        break;
    case 0xFF02:
        //SC -- SIO control (r/w)
        //std::cout << "SC!" <<std::endl;
        memory[offsetAddr] = value;
        break;
    case 0xFF04:
        //DIV -- Divider Register (r/w)
        memory[offsetAddr] = 0;
        break;
    case 0xFF05:
        //TIMA -- Timer Counter (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF06:
        //TMA -- Timer Modulo (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF07:
        //TAC -- Timer Control (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF0F:
        //IF -- Interrupt Flag (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF10 ... 0xFF3F:
        //Sound stuff
        memory[offsetAddr] = value;
        break;
    case 0xFF40:
        //LCDC -- LCD Control (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF41:
        //STAT -- LCDC Status (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF42:
        //SCY -- Scroll Y (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF43:
        //SCX -- Scroll X (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF44:
        //LY -- Scroll Y (r)
        throw std::runtime_error("Cannot write to LY @ 0xFF44");
    case 0xFF45:
        //LYC -- LY Compare (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF46:
        //DMA -- DMA Transfer and start address (W)
        throw std::runtime_error("DMA Transfer not implemented!");
    case 0xFF47 ... 0xFF49:
        //BGP -- BG & Window palette data (r/w)
        //OBP0 -- Object Palette 0 data (r/w)
        //OBP1 -- Object Palette 1 data (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF4A: case 0xFF4B:
        //Window x and y positions
        memory[offsetAddr] = value;
        break;
    
    default:
        throw std::runtime_error("Bad IO write address");
    }
}

uint8_t IO::read(uint16_t addr)
{
    uint8_t offsetAddr = addr-OFFSET_ADDR;
    switch (addr)
    {
    case 0xFF00:
        //P1 -- joypad info (r/w)
        return memory[offsetAddr];
    case 0xFF01:
        //SB -- Serial transfer data (r/w)
        return memory[offsetAddr];
    case 0xFF02:
        //SC -- SIO control (r/w)
        return memory[offsetAddr];
    case 0xFF04:
        //DIV -- Divider Register (r/w)
        return memory[offsetAddr];
    case 0xFF05:
        //TIMA -- Timer Counter (r/w)
        return memory[offsetAddr];
    case 0xFF06:
        //TMA -- Timer Modulo (r/w)
        return memory[offsetAddr];
    case 0xFF07:
        //TAC -- Timer Control (r/w)
        return memory[offsetAddr];
    case 0xFF0F:
        //IF -- Interrupt Flag (r/w)
        return memory[offsetAddr];
    case 0xFF10 ... 0xFF3F:
        //Sound stuff
        return memory[offsetAddr];
    case 0xFF40:
        //LCDC -- LCD Control (r/w)
        return memory[offsetAddr];
    case 0xFF41:
        //STAT -- LCDC Status (r/w)
        return memory[offsetAddr];
    case 0xFF42:
        //SCY -- Scroll Y (r/w)
        return memory[offsetAddr];
    case 0xFF43:
        //SCX -- Scroll X (r/w)
        return memory[offsetAddr];
    case 0xFF44:
        //LY -- Scroll Y (r)
        // TODO implement LY
        //std::cout << "LY read requested" << std::endl;
        return 0;
    case 0xFF45:
        //LYC -- LY Compare (r/w)
        return memory[offsetAddr];
    case 0xFF46:
        //DMA -- DMA Transfer and start address (W)
        throw std::runtime_error("DMA read not permitted!");
    case 0xFF47 ... 0xFF49:
        //BGP -- BG & Window palette data (r/w)
        //OBP0 -- Object Palette 0 data (r/w)
        //OBP1 -- Object Palette 1 data (r/w)
        return memory[offsetAddr];
    case 0xFF4A: case 0xFF4B:
        //Window x and y positions
        return memory[offsetAddr];
    
    default:
        std::cout << std::endl;
        throw std::runtime_error("Bad IO read address");
    }
}

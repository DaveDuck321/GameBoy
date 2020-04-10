#include "io_manager.hpp"

#include <iostream>

IO_Manager::IO_Manager()
{
    memory.fill(0xF4); //For debuging
}

uint8_t IO_Manager::vramRead(uint16_t addr) const
{
    switch(addr)
    {
        case 0x8000 ... 0x97FF:
            // Tile data 1
            // Flatten 3D array and write to it like the gameboy would
            // (Might cause errors on some compilers -- TODO: check)
            static_assert(sizeof(patternTables) == 0x1800);
            return (reinterpret_cast<const uint8_t *>(&patternTables))[addr-0x8000];
        case 0x9800 ... 0x9BFF:
            //Background map 1
            static_assert(sizeof(backgroundMap1) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap1))[addr-0x9800];
        case 0x9C00 ... 0x9FFF:
            //Background map 1
            static_assert(sizeof(backgroundMap2) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap2))[addr-0x9C00];
        default:
            throw std::range_error("Bad vram address read");
    }
}

void IO_Manager::vramWrite(uint16_t addr, uint8_t value)
{
    switch(addr)
    {
        case 0x8000 ... 0x97FF:
            // Tile data 1
            static_assert(sizeof(patternTables) == 0x1800);
            (reinterpret_cast<uint8_t *>(&patternTables))[addr-0x8000] = value;
            break;
        case 0x9800 ... 0x9BFF:
            //Background map 1
            static_assert(sizeof(backgroundMap1) == 0x400);
            (reinterpret_cast<uint8_t *>(&backgroundMap1))[addr-0x9800] = value;
            break;
        case 0x9C00 ... 0x9FFF:
            //Background map 2
            static_assert(sizeof(backgroundMap2) == 0x400);
            (reinterpret_cast<uint8_t *>(&backgroundMap2))[addr-0x9C00] = value;
            break;
        default:
            throw std::range_error("Bad vram address write");
    }
}

void IO_Manager::ioWrite(uint16_t addr, uint8_t value)
{
    uint8_t offsetAddr = addr-0xFF00;
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

uint8_t IO_Manager::ioRead(uint16_t addr) const
{
    uint8_t offsetAddr = addr-0xFF00;
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


void IO_Manager::draw() const
{
    // Allow implementation to initialize
    clearScreen();

    // Draw background 1
    for(uint8_t x=0; x<0x20; x++)
    {
        for(uint8_t y=0; y<0x20; y++)
        {
            uint8_t tileIndex = backgroundMap1[y][x];
            drawTile(patternTables[tileIndex], x*8, y*8);
        }
    }

    // For if implementation needs to finish rendering
    finishRender();
}

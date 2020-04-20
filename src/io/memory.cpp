#include "io_manager.hpp"

uint8_t IO_Manager::videoRead(uint16_t addr) const
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
            //Background map 2
            static_assert(sizeof(backgroundMap2) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap2))[addr-0x9C00];
        case 0xFE00 ... 0xFE9F:
            //Sprite attributes
            static_assert(sizeof(sprites) == 0xA0);
            return (reinterpret_cast<const uint8_t *>(&sprites))[addr-0xFE00];
        default:
            throw std::range_error("Bad vram address read");
    }
}

void IO_Manager::videoWrite(uint16_t addr, uint8_t value)
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
        case 0xFE00 ... 0xFE9F:
            //Sprite attributes
            static_assert(sizeof(sprites) == 0xA0);
            (reinterpret_cast<uint8_t *>(&sprites))[addr-0xFE00] = value;
            break;
        default:
            throw std::range_error("Bad vram address write");
    }
}

void IO_Manager::ioWrite(uint16_t addr, uint8_t value)
{
    switch (addr)
    {
    case 0xFF00:
        //P1 Joypad
        memory[addr-IO_OFFSET] = 0xC0|(value&0x30);
        break;
    case 0xFF02:
        //SC -- SIO control (r/w)
        // Immediately display serial data to console
        //std::cout<<memory[SERIAL_DATA];
        memory[addr-IO_OFFSET] = value;
        break;
    case 0xFF04:
        //DIV -- Divider Register (cannot write data)
        throw std::runtime_error("DIV write unsupported");
        break;
    case 0xFF05:
        // TIMA -- Timer counter (R/W)
        // Should only update timers between instructions when accessed
        updateTimers();
        memory[addr-IO_OFFSET] = value;
        break;
    
    // Special LCD registers
    case 0xFF41:
        // LCD Status Register
        memory[LCD_STAT] = 0x80|(memory[LCD_STAT]&0x07)|(value&0x78);
        break;
    case 0xFF44:
        //LY -- Scroll Y (r)
        throw std::runtime_error("Cannot write to LY @ 0xFF44");

    // Sound registers
    case 0xFF26:
        //NR52 - Sound on/off
        // All bits are read only except 7 (Sound on/off)
        if((value&0x80))    powerUpAPU();
        else                powerDownAPU();
        break;
    case 0xFF10 ... 0xFF25:
        // All Sound Control registers
        // Writing to registers should only be possible when APU is powered
        if(!(memory[NR52_REG]&0x80)) break;
        //Intentional fall through

    default:
        // Most IO actions don't require immediate action, deal with it later
        memory[addr-IO_OFFSET] = value;
        break;
    }
}

uint8_t IO_Manager::ioRead(uint16_t addr)
{
    switch(addr)
    {
    case 0xFF00: {
        // P1/JOYP -- Joypad read
        uint8_t value = 0xFF;
        if(!(memory[addr-IO_OFFSET]&0x10))
        {
            // P14 select and any p14 pressed keys are 0
            value &= 0xE0|(inputs&0x0F);
        }
        if(!(memory[addr-IO_OFFSET]&0x20))
        {
            // P15 select and any p15 pressed keys are 0
            value &= 0xD0|(inputs>>4);
        }
        return value;
    }
    case 0xFF46:
        // DMA - DMA Transfer and Start Address (W)
        // DMA reads are never allowed
        // There's probably an error somewhere else so throw
        throw std::runtime_error("DMA read not permitted!");

    //Sound
    case 0xFF10:
        // NR10 - Channel 1 Sweep register (R/W)
        return 0x80|memory[addr-IO_OFFSET];
    case 0xFF11: case 0xFF16:
        // NR11 - Channel 1 Sound length
        return 0x3F|memory[addr-IO_OFFSET];
    case 0xFF13: case 0xFF18: case 0xFF1B: case 0xFF1D: case 0xFF20:
        // NR13, NR23, NR31, NR33, NR41 (W)
        // All write only so ignore read request
        return 0xFF;
    case 0xFF14: case 0xFF19: case 0xFF23: case 0xFF1E:
        // NR14, NR24, NR44, NR34 (R/W)
        // All registers control channel frequency
        // Bit 0-2 and 7 are write only, bits 3-5 are invalid
        return 0xBF|memory[addr-IO_OFFSET];
    case 0xFF1A:
        // NR30 - Channel 3 Sound on/off (R/W)
        return 0x7F|memory[addr-IO_OFFSET];
    case 0xFF1C:
        // NR32 - Channel 3 Select output level (R/W)
        return 0x9F|memory[addr-IO_OFFSET];
    case 0xFF26:
        // NR52 - Sound on/off
        //TODO: real implementation
        return 0x70|memory[addr-IO_OFFSET];
    case 0xFF27 ... 0xFF2F: case 0xFF15: case 0xFF1F:
        // Invalid memory, should always return 0xFF
        return 0xFF;


    case 0xFF04: case 0xFF05:
        // Timer registers: DIV, TIMA
        // Should only update timers between instructions when accessed
        updateTimers(); // Timers lazy update
        //Intentional fall through

    default:
        return memory[addr-IO_OFFSET];
    }
}

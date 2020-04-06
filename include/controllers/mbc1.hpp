#ifndef mbc1_h
#define mbc1_h

#include "cartridge.hpp"

#include <memory>
#include <array>

class MBC1: public Controller
{
    private:
    //Possible memory modes are
    //  16Mbit ROM/8KByte RAM   (false)
    //  4Mbit ROM/32KByte RAM   (true)
    bool bankedRamMode = false;
    bool ramEnabled = true;

    // Both RAM and ROM banks can be selected by index
    uint8_t romBank = 1;
    uint8_t ramBank = 0;

    // Allocate enough ram for the full 32KByte RAM mode
    std::array<uint8_t, 0x8000> ram;

    public:
    MBC1(const std::vector<uint8_t>& rom): Controller(rom) {}

    uint8_t read(uint16_t addr) const override
    {
        switch (addr>>12)
        {
            // Always read from ROM bank 0 if addr < 0x4000
            case 0: case 1: case 2: case 3:
                return rom[addr];
            // Selectable rom banks from 0x4000 - 0x7FFF
            case 4: case 5: case 6: case 7:{
                uint16_t bankOffset = addr - 0x4000;
                return rom[(0x4000*romBank) + bankOffset];
            }
            // Cartridge RAM (Selectable in 32KByte RAM mode)
            case 0xA: case 0xB:{
                uint16_t bankOffset = addr - 0xA000;
                return ram[(0x2000*ramBank) + bankOffset];
            }
            default:
                throw std::runtime_error("Cannot read ROM address");
        }
    }

    void write(uint16_t addr, uint8_t value) override
    {
        switch (addr>>12)
        {
        // 0x0000 - 0x1FFF area disables RAM in 32KByte RAM mode
        case 0: case 1:
            if(!bankedRamMode) break;
            ramEnabled = (value&0xA) == 0xA;
            break;
        // 0x2000 - 0x3FFF area selects ROM bank
        case 2: case 3:
            if(value&0x1F)  romBank = value&0x1F;
            else            romBank = 1; //Cannot select bank 0
            break;

        // 0x4000 - 0x5FFF area selects either:
        //      ROM bank significant bits (in 16Mbit ROM)
        //      RAM bank                  (in 32KByte RAM mode)
        case 4: case 5:
            if(bankedRamMode)
                ramBank = value&0x3;
            else
                romBank = ( (value&0x3) << 5 ) | (romBank&0x1F);
            break;

        // 0x6000 - 0x7FFF area selects memory mode
        case 6: case 7:
            bankedRamMode = value&1;
            break;

        // 0xA000 - 0xBFFF area is the cartridge RAM.
        // This is a normal write unlike the other operations
        case 0xA: case 0xB:{
            uint16_t bankOffset = addr - 0xA000;
            ram[(0x2000*ramBank) + bankOffset] = value;
            break;
        }

        default:
            throw std::runtime_error("Cannot write to ROM");
        }
    }
};

#endif
#include "gb.hpp"

#include <iostream>

GB::GB(Cartridge &cartridge): cartridge(cartridge), registers(this)
{
    //Power up sequence (from http://bgb.bircd.org/pandocs.htm)
    registers.setU16(Register::AF, 0x01B0);
    registers.setU16(Register::BC, 0x0013);
    registers.setU16(Register::DE, 0x00D8);
    registers.setU16(Register::HL, 0x014D);
    registers.sp = 0xFFFE;
    registers.pc = 0x0100; //Start at $100
    registers.IME = true; //Start with interrupts enabled
    registers.halt = false; //Runs normally to begin with
    writeU8(0xFF05, 0x00); //TIMA
    writeU8(0xFF06, 0x00); //TMA
    writeU8(0xFF07, 0x00); //TAC
    writeU8(0xFF10, 0x80); //NR10
    writeU8(0xFF11, 0xBF); //NR11
    writeU8(0xFF12, 0xF3); //NR12
    writeU8(0xFF14, 0xBF); //NR14
    writeU8(0xFF16, 0x3F); //NR21
    writeU8(0xFF17, 0x00); //NR22
    writeU8(0xFF19, 0xBF); //NR24
    writeU8(0xFF1A, 0x7F); //NR30
    writeU8(0xFF1B, 0xFF); //NR31
    writeU8(0xFF1C, 0x9F); //NR32
    writeU8(0xFF1E, 0xBF); //NR33
    writeU8(0xFF20, 0xFF); //NR41
    writeU8(0xFF21, 0x00); //NR42
    writeU8(0xFF22, 0x00); //NR43
    writeU8(0xFF23, 0xBF); //NR30
    writeU8(0xFF24, 0x77); //NR50
    writeU8(0xFF25, 0xF3); //NR51
    //writeU8(0xFF26, 0xF1-GB, $F0-SGB //NR52
    writeU8(0xFF40, 0x91); //LCDC
    writeU8(0xFF42, 0x00); //SCY
    writeU8(0xFF43, 0x00); //SCX
    writeU8(0xFF45, 0x00); //LYC
    writeU8(0xFF47, 0xFC); //BGP
    writeU8(0xFF48, 0xFF); //OBP0
    writeU8(0xFF49, 0xFF); //OBP1
    writeU8(0xFF4A, 0x00); //WY
    writeU8(0xFF4B, 0x00); //WX
    writeU8(0xFFFF, 0x00); //IE
}

GB::~GB()
{

}

uint8_t GB::pcPopU8(bool debug)
{
    int8_t result = readU8(registers.pc++);
    //if(debug) std::cout << "0x" << (0xFF & result) << " ";
    return result;
}

uint16_t GB::pcPopU16()
{
    uint16_t result = readU16(registers.pc);
    //std::cout << "0x" << result << " ";
    registers.pc += 2;
    return result;
}

void GB::writeU8(uint16_t addr, uint8_t value)
{
    switch (addr)
    {
    case 0x0000 ... 0x7FFF:
        // Rom
        cartridge.read(addr);
        break;
    case 0x8000 ... 0x9FFF:
        // Video Ram
        throw std::runtime_error("Video not implemented");
        break;
    case 0xA000 ... 0xBFFF:
        // External ram
        throw std::runtime_error("External ram not implemented");
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
        IO[addr - 0xFF00] = value;
        break;
    case 0xFF80 ... 0xFFFE:
        // High ram
        stack[addr - 0xFF80] = value;
        break;
    case 0xFFFF:
        // Interrupt Enable Register
        break;
    default:
        std::cout << "Impossible memory address: " << addr << std::endl;
        throw std::runtime_error("Impossible memory address");
    }
}

void GB::writeU16(uint16_t addr, uint16_t value)
{
    writeU8(addr, value&0xFF);
    writeU8(addr+1, value>>8);
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
        throw std::runtime_error("Video not implemented");
        break;
    case 0xA000 ... 0xBFFF:
        // External ram
        throw std::runtime_error("External ram not implemented");
        break;
    case 0xC000 ... 0xDFFF:
        // Work ram 2
        return workingRam[addr-0xC000];
    case 0xE000 ... 0xFDFF:
        //0xC000 - 0xDDFF echo ram
        return workingRam[addr-0xE000];
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
        return IO[addr - 0xFF00];
    case 0xFF80 ... 0xFFFE:
        // High ram
        return stack[addr - 0xFF80];
    case 0xFFFF:
        // Interrupt Enable Register
        break;
    default:
        std::cout << "Impossible memory address: " << addr << std::endl;
        throw std::runtime_error("Impossible memory address");
    }
}

void GB::printFlags()
{
    std::cout << "Z: " << registers.getFlags(Flag::Z) << std::endl;
    std::cout << "N: " << registers.getFlags(Flag::N) << std::endl;
    std::cout << "H: " << registers.getFlags(Flag::H) << std::endl;
    std::cout << "C: " << registers.getFlags(Flag::C) << std::endl;
}

uint16_t GB::readU16(uint16_t addr) const
{
    return readU8(addr) + (readU8(addr+1)<<8);
}

int main()
{
    Cartridge card = Cartridge::loadRom("roms/test.gb");
    GB gb(card);
    std::cout << std::hex;
    for(int i = 0; i < 500; i++) {
        gb.step();
        //if(i%300 == 0)
        //{
            //Sending Vsync interrupt
            //gb.writeU8(0xFF0F, 0x01);
            //std::cout << "Sent interrupt" << std::endl;
        //}
    }
    return EXIT_SUCCESS;
}
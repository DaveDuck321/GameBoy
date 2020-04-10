#include "gb.hpp"
#include "displays/sdl_display.hpp"
#include <iostream>

GB::GB(Cartridge &cartridge, IO_Manager &io):
    cartridge(cartridge),
    io(io),
    registers(*this),
    memory(cartridge, io)
{

}

GB::~GB()
{

}

uint8_t GB::readU8(uint16_t addr)
{
    // Reads an 8-Bit value from 'addr'
    cycle++; // Under normal circumstances a read takes 1 cycle
    return memory.read(addr);
}

uint16_t GB::readU16(uint16_t addr)
{
    /*
    Reads a 16-Bit value from 'addr'.
        The least significant bit has address 'addr'
        The most significant bit has address 'addr'+1
    */
    return readU8(addr) + (readU8(addr+1)<<8);
}


void GB::writeU8(uint16_t addr, uint8_t value)
{
    // Writes an 8-Bit value to 'addr'
    cycle++; // Under normal circumstances a write takes 1 cycle
    memory.write(addr, value);
}

void GB::writeU16(uint16_t addr, uint16_t value)
{
    /*
    Writes a 16-Bit value to 'addr'.
        Writes the least significant bit to 'addr'
        Wrties the most significant bit to 'addr'+1
    */
    writeU8(addr, value&0xFF);
    writeU8(addr+1, value>>8);
}


uint8_t GB::nextU8()
{
    // Returns the 8-Bit value pointed to by the program counter, increments the counter
    return readU8(registers.pc++);
}

uint16_t GB::nextU16()
{
    // Returns the 16-Bit value pointed to by the program counter, increments the counter twice
    uint16_t result = readU16(registers.pc);
    registers.pc += 2;
    return result;
}

void GB::handleInterrupts()
{
    /*
    Uses CALL to execute an interrupt if it is both triggered and enabled.
    Interrupts (0xFF0F):
        BIT     INTERRUPT           CALL(ADDRESS)
        0       V-Blank             0x40
        1       LCDC Status         0x48
        2       Timer               0x50
        3       Serial              0x58
        4       P10-P13 -> Low      0x60
    */
    //Dont do anything if interrupts are globally disabled
    if(!registers.IME) return;

    uint8_t triggered = readU8(0xFFFF) & readU8(0xFF0F) & 0x1F;
    if(triggered)
    {
        registers.halt = false;
        registers.IME = false;
        if(triggered & 0x01)       CALL_nn(0x40);
        else if(triggered & 0x02)  CALL_nn(0x48);
        else if(triggered & 0x04)  CALL_nn(0x50);
        else if(triggered & 0x08)  CALL_nn(0x58);
        else if(triggered & 0x10)  CALL_nn(0x60);
    }
}

void GB::update()
{
    //Check for interrupts (if enabled)
    handleInterrupts();

    //Do nothing if waiting for interrupt
    if(registers.halt && registers.IME) return;

    // Run the next operation from the program counter
    nextOP();
}


int main( int argc, char *argv[] )
{
    SDL_Display display;
    Cartridge card = Cartridge::loadRom("tests/cpu_instrs/individual/03-op sp,hl.gb");
    GB gb(card, display);
    std::cout << std::hex;

    for(int i = 0; ; i++) {
        gb.update();
        if(i%10000 == 0)
        {
            gb.io.draw();
        }
    }
    return EXIT_SUCCESS;
}

#include "io_manager.hpp"

#include <algorithm>
#include <iostream>

void IO_Manager::reduceTimer(uint_fast16_t threshold)
{
    /*
    Reduces the tCycleCount to within 'threshold' by incrementing the timer
    If timer overflows, an interrupt is triggered
    */
    while(tCycleCount >= threshold)
    {
        tCycleCount -= threshold;

        // Inc counter and detect overflow
        if(((++memory[T_COUNTER])&0xFF) == 0)
        {
            memory[T_COUNTER] = memory[T_MODULO];
            memory[INTERRUPTS] |= TIMER_INTERRUPT;
        }
    }
}

void IO_Manager::updateTimers()
{
    uint64_t dt = cycle-lastCycle;
    lastCycle = cycle;

    //Inc timer by real cycle time
    vCycleCount += dt;
    tCycleCount += dt;
    //Timer increments every 64 cycles
    memory[DIV_TIMER] = (cycle / 64) % 0x100;
    switch(memory[T_CONTROL]&0x07)
    {
        case 0x04:
            //4096 Hz
            reduceTimer(256);
            break;
        case 0x05:
            //262144 Hz
            reduceTimer(4);
            break;
        case 0x06:
            //65536 Hz
            reduceTimer(16);
            break;
        case 0x07:
            // 16384 Hz
            reduceTimer(64);
            break;
        default:
            //Timer is disabled
            break;
    }
}

void IO_Manager::pressKey(Key key)
{
    /*
    Presses key and triggers interrupt.
    Only call once per keypress if interrupts are important
    */
    memory[INTERRUPTS] |= INPUT_INTERRUPT;
    inputs &= ~static_cast<uint8_t>(key);
}

void IO_Manager::releaseKey(Key key)
{
    /*
    Unpresses key, no interrupt is triggered
    */
    inputs |= static_cast<uint8_t>(key);
}


void IO_Manager::powerDownAPU()
{
    /*
    Powers down the APU and clears all related sound registers.
    This disables all writing until APU is powered up again 
    */
    memory[NR52_REG] &= 0x7F;
    std::fill(memory.begin()+NR10_REG, memory.begin()+NR52_REG, 0);
}

void IO_Manager::powerUpAPU()
{
    /*
    Powers up the APU while maintaining register values.
    This enables all writing.
    */
    memory[NR52_REG] |= 0x80;
}

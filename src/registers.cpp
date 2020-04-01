#include "registers.hpp"
#include "gb.hpp"

Flag operator | (Flag f1, Flag f2){
    return static_cast<Flag>(
        static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2)
    );
}

uint8_t CPURegisters::getFlags(Flag flag)
{
    return f & static_cast<uint8_t>(flag);
}

void CPURegisters::setFlags(Flag flag, bool set)
{
    uint8_t flag_val = static_cast<uint8_t>(flag);
    f = (f & ~flag_val) | (set*flag_val);
}

void CPURegisters::setFlags(Flag flag)
{
    f = f | static_cast<uint8_t>(flag);
}

void CPURegisters::resetFlags(Flag flag)
{
    f = f & ~static_cast<uint8_t>(flag);
}

uint8_t CPURegisters::getU8(Register r)
{
    switch(r)
    {
    case Register::A:
        return a;
    case Register::B:
        return b;
    case Register::C:
        return c;
    case Register::D:
        return d;
    case Register::E:
        return e;
    case Register::H:
        return h;
    case Register::L:
        return l;
    case Register::HL_ptr:
        return gb->readU8(getU16(Register::HL));
    case Register::BC_ptr:
        return gb->readU8(getU16(Register::BC));
    case Register::DE_ptr:
        return gb->readU8(getU16(Register::DE));
    }
    throw std::runtime_error("Bad register getU8 enum");
}

void CPURegisters::setU8(Register r, int8_t value)
{
    switch(r)
    {
    case Register::A:
        a = value;
        break;
    case Register::B:
        b = value;
        break;
    case Register::C:
        c = value;
        break;
    case Register::D:
        d = value;
        break;
    case Register::E:
        e = value;
        break;
    case Register::H:
        h = value;
        break;
    case Register::L:
        l = value;
        break;
    case Register::HL_ptr:
        gb->writeU8(getU16(Register::HL), value);
        break;
    default:
        throw std::runtime_error("Bad register setU8 enum");
    }
}

uint16_t CPURegisters::getU16(Register r)
{
    switch (r)
    {
    case Register::AF:
        return (a<<8)+f;
    case Register::BC:
        return (b<<8)+c;
    case Register::DE:
        return (d<<8)+e;
    case Register::HL:
        return (h<<8)+l;
    case Register::SP:
        return sp;
    case Register::PC:
        return pc;
    }
    throw std::runtime_error("Bad register getU16 enum");
}

void CPURegisters::setU16(Register r, int16_t value)
{
    switch (r)
    {
    case Register::AF:
        a = value>>8;
        f = value&0xFF;
        break;
    case Register::BC:
        b = value>>8;
        c = value&0xFF;
        break;
    case Register::DE:
        d = value>>8;
        e = value&0xFF;
        break;
    case Register::HL:
        h = value>>8;
        l = value&0xFF;
        break;
    case Register::SP:
        sp = value;
        break;
    case Register::PC:
        pc = value;
        break;
    default:
        throw std::runtime_error("Bad register setU16 enum");
    }
}
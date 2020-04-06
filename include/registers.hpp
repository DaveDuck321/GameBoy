#ifndef registers_hpp
#define registers_hpp

#include <type_traits>
#include <vector>
#include <iostream>

enum class Register
{
    A, B, C, D, E, H, L,
    AF, BC, DE, HL,
    HL_ptr, BC_ptr, DE_ptr,
    SP, PC
};

enum class Flag : uint8_t
{
    C = 0x10,
    H = 0x20,
    N = 0x40,
    Z = 0x80
};

// Copy is fine here
Flag operator | (Flag f1, Flag f2);

//Dont really like this, but it makes everything else prettier
//GB is used for pointer registers
class GB;
class CPURegisters
{
    private:
    GB *gb; // For resolving pointer registers

    public:
    uint8_t a, b, c, d, e, f, h, l;
    uint16_t sp, pc;
    bool IME, halt;

    CPURegisters(GB *gb);

    void printFlags() const;
    void printRegs() const;
    bool getFlags(Flag flag) const;

    void setFlags(Flag flag, bool set);
    void setFlags(Flag flag);
    void resetFlags(Flag flag);

    uint8_t getU8(Register r) const;
    uint16_t getU16(Register r) const;


    void setU8(Register r, int8_t value);
    void setU16(Register r, int16_t value);
};

#endif
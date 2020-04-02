#include "gb.hpp"

void GB::LD_n_nn(Register n, uint8_t nn)
{
    //TODO: Check this very strange convention
    /*
    Description:
        Put value nn into n.
    Use with:
        n = B,C,D,E,H,L  nn = 8 bit immediate value
    */
    registers.setU8(n, nn);
    //write(n, registers.getU8(nn));
}

void GB::LD_r1_r2(Register r1, Register r2)
{
    /*
    Description:
        Put value r2 into r1.
    Use with:
        r1 = A,B,C,D,E,H,L,(HL)
        r2 = A,B,C,D,E,H,L,(HL),n
    */
    registers.setU8(r1, registers.getU8(r2));
}

void GB::LD_r_A(Register r)
{
    /*
    Description:
        Put value A into n (registers).
    Use with:
        n = A,B,C,D,E,H,L
    */
    registers.setU8(r, registers.a);
}
void GB::LD_n_A(uint8_t addr)
{
    /*
    Description:
        Put value A into n (memory).
    Use with:
        n = (BC),(DE),(HL),(nn)
        nn = two byte immediate value. (LS byte first.)
    */
    writeU8(addr, registers.a);
}

void GB::LD_A_C()
{
    /*
    Description:
        Put value at address $FF00 + register C into A.
    Same as:
        LD A,($FF00+C)
    */
    registers.a = readU8(0xFF00 + registers.c);
}

void GB::LD_C_A()
{
    /*
    Description:
        Put A into address $FF00 + register C.
    */
    writeU8(0xFF00 + registers.c, registers.a);
}

void GB::LDD_A_HL()
{
    /*
    Description:
        Put value at address HL into A. Decrement HL.
    Same as:
        LD A,(HL) - DEC HL
    */
    uint16_t hl = registers.getU16(Register::HL);
    registers.a = readU8(hl);
    DEC16_nn(Register::HL);
}

void GB::LDD_HL_A()
{
    /*
    Description:
        Put A into memory address HL. Decrement HL.
    Same as:
        LD (HL),A - DEC HL
    */
    uint16_t hl = registers.getU16(Register::HL);
    writeU8(hl, registers.a);
    DEC16_nn(Register::HL);
}

void GB::LDI_A_HL()
{
    /*
    Description:
        Put value at address HL into A. Increment HL.
    Same as:
        LD A,(HL) - INC HL
    */
    uint16_t hl = registers.getU16(Register::HL);
    registers.a = readU8(hl);
    INC16_nn(Register::HL);
}

void GB::LDI_HL_A()
{
    /*
    Description:
        Put A into memory address HL. Decrement HL.
    Same as:
        LD (HL),A - INC HL
    */
    uint16_t hl = registers.getU16(Register::HL);
    writeU8(hl, registers.a);
    INC16_nn(Register::HL);
}

void GB::LDH_n_A(uint8_t n)
{
    /*
    Description:
        Put A into memory address $FF00+n.
    Use with:
        n = one byte immediate value
    */
    writeU8(0xFF00+n, registers.a);
}

void GB::LDH_A_n(uint8_t n)
{
    /*
    Description:
        Put memory address $FF00+n into A.
    Use with:
        n = one byte immediate value
    */
    registers.a = readU8(0xFF00+n);
}

void GB::LD16_n_nn(Register n, uint16_t nn)
{
    /*
    Description:
        Put value nn into n.
    Use with:
        n = BC,DE,HL,SP  nn = 16 bit immediate value
    */
    registers.setU16(n, nn);
}

void GB::LD16_SP_HL()
{
    /*
    Description:
        Put HL into Stack Pointer (SP)
    */
    registers.sp = registers.getU16(Register::HL);
}

void GB::LD16_SP_n(int8_t n)
{
    /*
    Description:
        Put SP + n effective address into HL.
    Use with:
        n = one byte signed immediate value.
    Flags affected:
        Z - Reset.
        N - Reset.
        H - Set or reset according to operation.
        C - Set or reset according to operation.
    */
    registers.setU16(Register::HL, registers.sp+n);
    registers.resetFlags(Flag::Z|Flag::N);

    registers.setFlags(Flag::C, n > 0xFFFF-registers.sp);
    registers.setFlags(Flag::H, (n&0x00FF) > (0x00FF - (registers.sp&0x00FF)));
    //TODO: debug potential error here, flags for carry look wrong
}

void GB::LD_nn_SP(int16_t nn)
{
    /*
    Description:
        Put Stack Pointer (SP) at address n.
    Use with:
        nn = two byte immediate address
    */
    registers.sp = nn;
}

void GB::PUSH(Register r)
{
    /*
    Description:
        Push register pair nn onto stack.
        Decrement Stack Pointer (SP) twice.
    Use with:
        nn = AF,BC,DE,HL
    */
    //TODO: check order
    registers.sp -= 2;
    writeU16(registers.sp, registers.getU16(r));
}

void GB::POP(Register r)
{
    /*
    Description:
        Pop two bytes off stack into register pair nn.
        Increment Stack Pointer (SP) twice.
    Use with:
        nn = AF,BC,DE,HL
    */
    //TODO: check order
    registers.setU16(r, readU16(registers.sp));
    registers.sp += 2;

}

void GB::ADD_n(uint8_t n)
{
    //TODO: guessed C flag operation, might be wrong
    /*
    Description:
        Add n + Carry flag to A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Set if carry from bit 3.
        C - Set if carry from bit 7
    */
    registers.setFlags(Flag::C, n > 0xFF - registers.a);
    registers.setFlags(Flag::H, (n&0x0F) > (0x0F - (registers.a&0x0F)));
    registers.a = registers.a+n;
    registers.setFlags(Flag::Z, registers.a == 0);
    registers.resetFlags(Flag::N);
}

void GB::ADC_n(uint8_t n)
{
    /*
    Description:
        Add n + Carry flag to A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Set if carry from bit 3.
        C - Set if carry from bit 7
    */
    ADD_n(n + registers.getFlags(Flag::C));
}

void GB::SUB_n(uint8_t n)
{
    //TODO: guessed C flag operation, might be wrong
    /*
    Description:
        Subtract n from A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Set.
        H - Set if no borrow from bit 4.
        C - Set if no borrow.
    */
    registers.setFlags(Flag::C, registers.a - n < 0);
    registers.setFlags(Flag::H, (registers.a&0x0F) - (n&0x0F) < 0);

    registers.a = registers.a+n;
    registers.setFlags(Flag::Z, registers.a == 0);
    registers.setFlags(Flag::N);
}

void GB::SBC_n(uint8_t n)
{
    /*
    Description:
        Subtract n + Carry flag from A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Set.
        H - Set if no borrow from bit 4.
        C - Set if no borrow
    */
    SUB_n(n + registers.getFlags(Flag::C));
}

void GB::AND_n(uint8_t n)
{
    /*
    Description:
        Logically AND n with A, result in A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Set.
        C - Reset.
    */
    registers.a = registers.a & n;
    registers.setFlags(Flag::Z, registers.a==0);
    registers.setFlags(Flag::H);
    registers.resetFlags(Flag::N|Flag::C);
}

void GB::OR_n(uint8_t n)
{
    /*
    Description:
        Logical OR n with register A, result in A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Reset
    */
    registers.a = registers.a | n;
    registers.setFlags(Flag::Z, registers.a==0);
    registers.resetFlags(Flag::H|Flag::N|Flag::C);
}

void GB::XOR_n(uint8_t n)
{
    /*
    Description:
        Logical exclusive OR n with register A, result in A.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Reset
    */
    registers.a = registers.a ^ n;
    registers.setFlags(Flag::Z, registers.a==0);
    registers.resetFlags(Flag::H|Flag::N|Flag::C);
}

void GB::CP_n(uint8_t n)
{
    /*
    Description:
        Compare A with n. This is basically an A - n subtraction instruction but the results are thrown  away.
    Use with:
        n = A,B,C,D,E,H,L,(HL),#
    Flags affected:
        Z - Set if result is zero. (Set if A = n.)
        N - Set.
        H - Set if no borrow from bit 4.
        C - Set for no borrow. (Set if A < n.)
    */
    registers.setFlags(Flag::Z, registers.a==n);
    registers.setFlags(Flag::N);
    registers.setFlags(Flag::H, (registers.a&0x0F) < (n&0x0F));
    registers.setFlags(Flag::C, registers.a < n);
}

void GB::INC_r(Register r)
{
    /*
    Description:
        Increment register n.
    Use with:
        n = A,B,C,D,E,H,L
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Set if carry from bit 3.
        C - Not affected.
    */
    uint8_t result = registers.getU8(r)+1;
    registers.setU8(r, result);
    registers.setFlags(Flag::Z, result==0);
    registers.setFlags(Flag::H, result&0x0F == 0);
    registers.resetFlags(Flag::N);
}

void GB::DEC_r(Register r)
{
    /*
    Description:
        Decrement register n.
    Use with:
        n = A,B,C,D,E,H,L,(HL)
    Flags affected:
        Z - Set if reselt is zero.
        N - Set.
        H - Set if no borrow from bit 4.
        C - Not affected.
    */
    //TODO: Set if no borrow from bit 4.
    uint8_t result = registers.getU8(r)-1;
    registers.setU8(r, result);
    registers.setFlags(Flag::Z, result==0);
    registers.setFlags(Flag::H, result&0x0F != 0);
    registers.setFlags(Flag::N);
}

uint16_t GB::ADD16(uint16_t n1, uint16_t n2)
{
    /*
    Description:
        Add n1 to n2.
    Use with:
        n1, n2 = 16bit values
    Flags affected:
        N - Reset.
        H - Set if carry from bit 11.
        C - Set if carry from bit 15. 
    */
    registers.resetFlags(Flag::N);
    registers.setFlags(Flag::H, (n2&0x400) > 0x400 - (n1&0x400));
    registers.setFlags(Flag::C, n2 > 0xFFFF - n1);
    return n1+n2;
}


void GB::ADD16_HL_n(Register n)
{
    /*
    Description:
        Add n to HL.
    Use with:
        n = BC,DE,HL,SP
    Flags affected:
        Z - Not affected.
        N - Reset.
        H - Set if carry from bit 11.
        C - Set if carry from bit 15. 
    */
   // TODO check H
    uint16_t hl_val = registers.getU16(Register::HL);
    uint16_t n_val = registers.getU16(n);
    registers.setU16(Register::HL, ADD16(hl_val, n_val));
}

void GB::ADD16_SP_n(uint8_t n)
{
    /*
    Description:
        Add n to Stack Pointer (SP).
    Use with:
        n = one byte signed immediate value (#).
    Flags affected:
        Z - Reset.
        N - Reset.
        H - Set or reset according to operation.
        C - Set or reset according to operation.
    */
    registers.sp = ADD16(registers.sp, n);
    registers.resetFlags(Flag::Z);
}

void GB::INC16_nn(Register nn)
{
    /*
    Description:
        Increment register nn.
    Use with:
        nn = BC,DE,HL,SP
    Flags affected:
        None
    */
    registers.setU16(nn, registers.getU16(nn)+1);
}

void GB::DEC16_nn(Register nn)
{
    /*
    Description:
        Decrement register nn.
    Use with:
        nn = BC,DE,HL,SP
    Flags affected:
        None
    */
   registers.setU16(nn, registers.getU16(nn)-1);
}

void GB::SWAP_n(Register n)
{
    /*
    Description:
        Swap upper & lower nibles of n.
    Use with:
        n = A,B,C,D,E,H,L
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Reset
    */
    uint8_t value = registers.getU8(n);
    registers.setFlags(Flag::Z, value==0);
    registers.resetFlags(Flag::N|Flag::H|Flag::C);
    registers.setU8(n, (value&0x0F)<<8 + (value&0xF0)>>8);
}

void GB::DAA()
{
    /*
    Description:
        Decimal adjust register A.
        This instruction adjusts register A so that the correct representation of Binary Coded Decimal (BCD)  is obtained.
    Flags affected:
        Z - Set if register A is zero.
        N - Not affected.
        H - Reset.
        C - Set or reset according to operation.
    */
    throw std::runtime_error("DAA Not implemented");
}

void GB::CPL()
{
    /*
    Description:
        Complement A register. (Flip all bits.)
    Flags affected:
        Z - Not affected.
        N - Set.
        H - Set.
        C - Not affected.
    */
    registers.setFlags(Flag::N|Flag::H);
    registers.a = ~registers.a;
}

void GB::CCF()
{
    /*
    Description:
        Complement carry flag.
        If C flag is set, then reset it.
        If C flag is reset, then set it.
    Flags affected:
        Z - Not affected.
        N - Reset.
        H - Reset.
        C - Complemented.
    */
    registers.resetFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::C, !registers.getFlags(Flag::C));
}

void GB::SCF()
{
    /*
    Description:
        Set Carry flag.
    Flags affected:
        Z - Not affected.
        N - Reset.
        H - Reset.
        C - Set.
    */
    registers.resetFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::C);
}

void GB::NOP()
{
    /*
    Description:
        No operation
    */
}

void GB::HALT()
{
    /*
    Description:
        Power down CPU until an interrupt occurs.
        Use this when ever possible to reduce energy consumption.
    */
    registers.halt = true;
}

void GB::STOP()
{
    /*
    Description:
        Halt CPU & LCD display until button pressed
    */
    throw std::runtime_error("STOP Not implemented");
}

void GB::DI()
{
    /*
    Description:
        This instruction disables interrupts but not immediately.
        Interrupts are disabled after instruction after DI is executed.
    Flags affected:
        None.
    */
    registers.IME = false; //TODO: Timing is wrong here
}

void GB::EI()
{
    /*
    Description:
        Enable interrupts. This intruction enables interrupts but not immediately.
        Interrupts are enabled after instruction after EI is executed.
    Flags affected:
        None.
    */
    registers.IME = true; //TODO: Timing is wrong here
}

uint8_t GB::ROT_LC(uint8_t value)
{
    /*
    Description:
        Rotate value left. Old bit 7 to Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
    */
    uint8_t result = (value << 1) + ((value&0x80) >> 7);

    registers.resetFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::C, value&0x80);
    //registers.setFlags(Flag::Z, result==0);
    registers.resetFlags(Flag::Z);

    return result;
}

uint8_t GB::ROT_L(uint8_t value)
{
    /*
    Description:
        Rotate value left through Carry flag
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
    */
    bool carryFlag = registers.getFlags(Flag::C);
    uint8_t result = (value << 1) + carryFlag;

    registers.resetFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::C, value&0x80);
    registers.setFlags(Flag::Z, result==0);
    return result;
}

uint8_t GB::ROT_RC(uint8_t value)
{
    /*
    Description:
        Rotate value right. Old bit 0 to Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 0 data
    */
    uint8_t result = (value >> 1) + ((value&1) << 7);

    registers.resetFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::C, value&1);
    //registers.setFlags(Flag::Z, result==0);
    registers.resetFlags(Flag::Z);

    return result;
}

uint8_t GB::ROT_R(uint8_t value)
{
    /*
    Description:
        Rotate value right through Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 0 data
    */
    bool carryFlag = registers.getFlags(Flag::C);
    uint8_t result = (value >> 1) + (carryFlag << 7);

    registers.resetFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::C, value&1);
    registers.setFlags(Flag::Z, result==0);
    return result;
}

void GB::RLCA()
{
    /*
    Description:
        Rotate A left. Old bit 7 to Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
    */
    registers.a = ROT_LC(registers.a);
}

void GB::RLA()
{
    /*
    Description:
        Rotate A left through Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data
    */
    registers.a = ROT_L(registers.a);
}

void GB::RRCA()
{
    /*
    Description:
        Rotate A right. Old bit 0 to Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 0 data
    */
    registers.a = ROT_RC(registers.a);
}

void GB::RRA()
{
    /*
    Description:
        Rotate A right through Carry flag.
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 0 data
    */
    registers.a = ROT_R(registers.a);
}

void GB::RLC_r(Register r)
{
    /*
    Description:
        Rotate n left. Old bit 7 to Carry flag.
    Use with:
        n = A,B,C,D,E,H,L,(HL)
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
    */
    registers.setU8(r, ROT_LC(registers.getU8(r)));
}

void GB::RL_r(Register r)
{
    /*
    Description:
        Rotate n left through Carry flag.
    Use with:
        n = A,B,C,D,E,H,L, (HL)
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
    */
    registers.setU8(r, ROT_L(registers.getU8(r)));
}

void GB::RRC_r(Register r)
{
    /*
    Description:
        Rotate n right. Old bit 0 to Carry flag.
    Use with:
        n = A,B,C,D,E,H,L, (HL)
    Flags affected:
        Z - Set if result is zero.
        N - Reset.  H - Reset.
        C - Contains old bit 0 data.
    */
   registers.setU8(r, ROT_RC(registers.getU8(r)));
}

void GB::RR_r(Register r)
{
    /*
    Description:
        Rotate n right through Carry flag.
    Use with:
        n = A,B,C,D,E,H,L
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 0 data.
    */
    registers.setU8(r, ROT_R(registers.getU8(r)));
}

void GB::SLA_n(Register r)
{
    /*
    Description:
        Shift n left into Carry. LSB of n set to 0.
    Use with:
        n = A,B,C,D,E,H,L,(HL)
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 7 data.
    */
    int8_t value = registers.getU8(r);
    registers.setU8(r, value<<1);
    registers.setFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::Z, (value&0x7f) == 0);
    registers.setFlags(Flag::C, value&0x80);
}

void GB::SRA_n(Register r)
{
    /*
    Description:
        Shift n right into Carry. MSB doesn't change.
    Use with:
        n = A,B,C,D,E,H,L,(HL)
    Flags affected:
        Z - Set if result is zero.
            N - Reset.
            H - Reset.
            C - Contains old bit 0 data
    */
    int8_t value = registers.getU8(r);
    int8_t result = (value>>1) | (value&0x80);
    registers.setFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::Z, result == 0);
    registers.setFlags(Flag::C, value&1);
    registers.setU8(r, result);
}

void GB::SRL_n(Register r)
{
    /*
    Description:
        Shift n right into Carry. MSB set to 0.
    Use with:
        n = A,B,C,D,E,H,L,(HL)
    Flags affected:
        Z - Set if result is zero.
        N - Reset.
        H - Reset.
        C - Contains old bit 0 data.
    */
    int8_t value = registers.getU8(r);
    int8_t result = (value>>1);
    registers.setFlags(Flag::N|Flag::H);
    registers.setFlags(Flag::Z, result == 0);
    registers.setFlags(Flag::C, value&1);
    registers.setU8(r, result);
}

void GB::BIT_b_r(uint8_t b, Register r)
{
    //TODO should analyse opcodes, unsure about bit widths here
    /*
    Description:
        Test bit b in register r.
    Use with:
        b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
    Flags affected:
        Z - Set if bit b of register r is 0.
        N - Reset.
        H - Set.
        C - Not affected.
    */
    registers.setFlags(Flag::Z, registers.getU8(r) & (1<<b));
    registers.setFlags(Flag::H);
    registers.resetFlags(Flag::N);
}

void GB::SET_b_r(uint8_t b, Register r)
{
    /*
    Description:
        Set bit b in register r.
    Use with:
        b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
    Flags affected:
        None
    */
    registers.setU8(r, registers.getU8(r) | (1<<b));
}

void GB::RES_b_r(uint8_t b, Register r)
{
    /*
    Description:
        Reset bit b in register r.
    Use with:
        b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
    Flags affected:
        None
    */
    registers.setU8(r, registers.getU8(r) & ~(1<<b));
}

void GB::JP_nn(uint16_t nn)
{
    /*
    Description:
        Jump to address nn.
    Use with:
        nn = two byte immediate value. (LS byte first.)
    */
   registers.pc = nn;
}

void GB::JP_cc_nn(Flag f, bool set, uint16_t nn)
{
    /*
    Description:
        Jump to address n if following condition is true:
        cc = NZ, Jump if Z flag is reset.
        cc = Z,  Jump if Z flag is set.
        cc = NC, Jump if C flag is reset.
        cc = C,  Jump if C flag is set.
    Use with:
        nn = two byte immediate value. (LS byte first.)
    */
    if(registers.getFlags(f) == set) JP_nn(nn);
}

void GB::JP_HL()
{
    /*
    Description:
        Jump to address contained in HL
    */
    JP_nn(registers.getU16(Register::HL));
}

void GB::JR_n(uint8_t n)
{
    /*
    Description:
        Add n to current address and jump to it.
    Use with:
        n = one byte signed immediate value
    */
    JP_nn(registers.pc+n);
}

void GB::JR_cc_n(Flag f, bool set, uint8_t n)
{
    /*
    Description:
        If following condition is true then add n to current address and jump to it:
    Use with:
        n = one byte signed immediate value
        cc = NZ, Jump if Z flag is reset.
        cc = Z,  Jump if Z flag is set.
        cc = NC, Jump if C flag is reset.
        cc = C,  Jump if C flag is set.
    */
    JP_cc_nn(f, set, registers.pc+n);
}

void GB::CALL_nn(int16_t nn)
{
    /*
    Description:
        Push address of next instruction onto stack and then  jump to address nn.
    Use with:
        nn = two byte immediate value. (LS byte first.)
    */

    PUSH(Register::PC);
    JP_nn(nn);
}

void GB::CALL_cc_nn(Flag f, bool set, uint16_t nn)
{
    /*
    Description:
        Call address n if following condition is true:
        cc = NZ, Call if Z flag is reset.
        cc = Z,  Call if Z flag is set.
        cc = NC, Call if C flag is reset.
        cc = C,  Call if C flag is set.
    Use with:
        nn = two byte immediate value. (LS byte first.)
    */
    if(registers.getFlags(f) == set) CALL_nn(nn);
}

void GB::RST_n(int8_t n)
{
    /*
    Description:
        Push present address onto stack.
        Jump to address $0000 + n.
    Use with:
        n = $00,$08,$10,$18,$20,$28,$30,$38
    */
    PUSH(Register::PC);
    JP_nn(n);
}

void GB::RET()
{
    /*
    Description:
        Pop two bytes from stack & jump to that address
    */
    uint16_t addr = readU16(registers.sp);
    registers.sp += 2;
    JP_nn(addr);
}

void GB::RET_cc(Flag f, bool set)
{
    /*
    Description:
        Return if following condition is true:
    Use with:
        cc = NZ, Return if Z flag is reset.
        cc = Z,  Return if Z flag is set.
        cc = NC, Return if C flag is reset.
        cc = C,  Return if C flag is set.
    */
    if(registers.getFlags(f) == set) RET();
}

void GB::RETI()
{
    /*
    Description:
        Pop two bytes from stack & jump to that address then enable interrupts
    */
    RET();
    EI();
}
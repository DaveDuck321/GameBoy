#include "gb.hpp"
#include <array>

const std::array<Register, 8> registerOpcodes = {
    Register::B, Register::C, Register::D, Register::E, Register::H, Register::L, Register::HL_ptr, Register::A
};

const std::array<Register, 4> register16Opcodes = {
    Register::BC, Register::DE, Register::HL, Register::SP
};

void GB::step()
{
    uint8_t opcode = pcPopU8();
    switch(opcode)
    {
    //8-Bit loads
        //LD nn,n
        case 0x06:
            LD_n_nn(Register::B, pcPopU8());
            break;
        case 0x0E:
            LD_n_nn(Register::C, pcPopU8());
            break;
        case 0x16:
            LD_n_nn(Register::D, pcPopU8());
            break;
        case 0x1E:
            LD_n_nn(Register::E, pcPopU8());
            break;
        case 0x26:
            LD_n_nn(Register::H, pcPopU8());
            break;
        case 0x2E:
            LD_n_nn(Register::L, pcPopU8());
            break;
        case 0x36:
            LD_n_nn(Register::HL_ptr, pcPopU8());
            break;
        //LD r1,r2
        case 0x78 ... 0x7F: //LD a, n
            LD_r1_r2(Register::A, registerOpcodes[opcode-0x78]);
            break;
        case 0x40 ... 0x46: //LD b, n
            LD_r1_r2(Register::B, registerOpcodes[opcode-0x40]);
            break;
        case 0x48 ... 0x4E: //LD c, n
            LD_r1_r2(Register::C, registerOpcodes[opcode-0x48]);
            break;
        case 0x50 ... 0x56: //LD d, n
            LD_r1_r2(Register::D, registerOpcodes[opcode-0x50]);
            break;
        case 0x58 ... 0x5E: //LD e, n
            LD_r1_r2(Register::E, registerOpcodes[opcode-0x58]);
            break;
        case 0x60 ... 0x66: //LD h, n
            LD_r1_r2(Register::H, registerOpcodes[opcode-0x60]);
            break;
        case 0x68 ... 0x6E: //LD l, n
            LD_r1_r2(Register::L, registerOpcodes[opcode-0x68]);
            break;
        case 0x70 ... 0x75: //LD (HL), n
            LD_r1_r2(Register::HL_ptr, registerOpcodes[opcode-0x70]);
            break;
        //LD A,n
        case 0x0A: //LD A, (BC)
            LD_r1_r2(Register::A, Register::BC_ptr);
            break;
        case 0x1A: //LD A, (DE)
            LD_r1_r2(Register::A, Register::DE_ptr);
            break;
        case 0xFA: //LD A, (nn)
            LD_n_nn(Register::A, pcPopU16());
            break;
        case 0x3E: //LD A, #
            LD_n_nn(Register::A, pcPopU8());
            break;
        //LD n, A
        case 0X47: //LD B,A
            LD_r_A(Register::B);
            break;
        case 0X4F: //LD C,A
            LD_r_A(Register::C);
            break;
        case 0X57: //LD D,A
            LD_r_A(Register::D);
            break;
        case 0X5F: //LD E,A
            LD_r_A(Register::E);
            break;
        case 0X67: //LD H,A
            LD_r_A(Register::H);
            break;
        case 0X6F: //LD L,A
            LD_r_A(Register::L);
            break;
        case 0x02: //LD (BC),A
            LD_r_A(Register::BC_ptr);
            break;
        case 0X12: //LD (DE),A
            LD_r_A(Register::DE_ptr);
            break;
        case 0x77: //LD (HL),A
            LD_r_A(Register::HL_ptr);
            break;
        case 0xEA: //LD (nn),A
            LD_n_A(pcPopU16());
            break;
        case 0xF2: //LD A,(C)
            LD_A_C();
            break;
        case 0xE2: //LD (C),A
            LD_C_A();
            break;
        case 0x3A: //LDD A,(Hl)
            LDD_A_HL();
            break;
        case 0x32: //LDD (HL),A
            LDD_HL_A();
            break;
        case 0x2A: //LDI A,(HL)
            LDI_A_HL();
            break;
        case 0x22: //LDI (HL),A
            LDI_HL_A();
            break;
        case 0xE0: //LDH (n),A
            LDH_n_A(pcPopU8());
            break;
        case 0xF0: //LDH A,(n)
            LDH_A_n(pcPopU8());
            break;
    //16-Bit loads
        //LD n,nn
        case 0x01: case 0x11: case 0x21: case 0x31:
            LD16_n_nn(register16Opcodes[opcode>>8], pcPopU16());
            break;
        case 0xF9: //LD SP,HL
            LD16_SP_HL();
            break;
        case 0xF8: //LDHL SP,n
            LD16_SP_n(pcPopU8());
            break;
        case 0x08: //LD (nn),SP
            LD_nn_SP(pcPopU16());
            break;
        //PUSH
        case 0xF5: //PUSH AF
            PUSH(Register::AF);
            break;
        case 0xC5: //PUSH BC
            PUSH(Register::BC);
            break;
        case 0xD5: //PUSH DE
            PUSH(Register::DE);
            break;
        case 0xE5: //PUSH HL
            PUSH(Register::HL);
            break;
        //POP
        case 0xF1: //POP AF
            POP(Register::AF);
            break;
        case 0xC1: //POP BC
            POP(Register::BC);
            break;
        case 0xD1: //POP DE
            POP(Register::DE);
            break;
        case 0xE1: //POP HL
            POP(Register::HL);
            break;
    //8-Bit ALU
        //ADD
        case 0x80 ... 0x87: //ADD A, n
            ADD_n(registers.getU8(registerOpcodes[opcode-0x80]));
            break;
        case 0xC6: // Add A, #
            ADD_n(pcPopU8());
            break;
        //ADC
        case 0x88 ... 0x8F: //ADC A, n
            ADC_n(registers.getU8(registerOpcodes[opcode-0x88]));
            break;
        case 0xCE: //ADC A, #
            ADC_n(pcPopU8());
            break;
        //SUB
        case 0x90 ... 0x97: //SUB n
            SUB_n(registers.getU8(registerOpcodes[opcode-0x90]));
            break;
        case 0xD6: //SUB #
            SUB_n(pcPopU8());
            break;
        //SBC
        case 0x98 ... 0x9F: //SBC A,n
            SBC_n(registers.getU8(registerOpcodes[opcode-0x98]));
            break;
        //AND
        case 0xA0 ... 0xA7: //AND n
            ADD_n(registers.getU8(registerOpcodes[opcode-0xA0]));
            break;
        case 0xE6: //AND #
            AND_n(pcPopU8());
        //OR
        case 0xB0 ... 0xB7: //OR n
            OR_n(registers.getU8(registerOpcodes[opcode-0xB0]));
            break;
        case 0xF6: //OR #
            OR_n(pcPopU8());
        //XOR
        case 0xA8 ... 0xAF: //XOR n
            XOR_n(registers.getU8(registerOpcodes[opcode-0xA8]));
            break;
        case 0xEE: //XOR #
            XOR_n(pcPopU8());
        //CP
        case 0xB8 ... 0xBF: //CP n
            XOR_n(registers.getU8(registerOpcodes[opcode-0xB8]));
            break;
        case 0xFE: //CP #
            XOR_n(pcPopU8());
        
        //INC
        case 0x3C: //INC A
            INC_r(Register::A);
            break;
        case 0x04: //INC B
            INC_r(Register::B);
            break;
        case 0x0C: //INC C
            INC_r(Register::C);
            break;
        case 0x14: //INC D
            INC_r(Register::D);
            break;
        case 0x1C: //INC E
            INC_r(Register::E);
            break;
        case 0x24: //INC H
            INC_r(Register::H);
            break;
        case 0x2C: //INC L
            INC_r(Register::L);
            break;
        case 0x34: //INC (HL)
            INC_r(Register::HL_ptr);
            break;
        //DEC
        case 0x3D: //DEC A
            DEC_r(Register::A);
            break;
        case 0x05: //DEC B
            DEC_r(Register::B);
            break;
        case 0x0D: //DEC C
            DEC_r(Register::C);
            break;
        case 0x15: //DEC D
            DEC_r(Register::D);
            break;
        case 0x1D: //DEC E
            DEC_r(Register::E);
            break;
        case 0x25: //DEC H
            DEC_r(Register::H);
            break;
        case 0x2D: //DEC L
            DEC_r(Register::L);
            break;
        case 0x35: //DEC (HL)
            DEC_r(Register::HL_ptr);
            break;
    //16-Bit ALU
        //ADD HL,n
        case 0x09: case 0x19: case 0x29: case 0x39:
            ADD16_HL_n(register16Opcodes[opcode>>8]);
            break;
        //ADD SP,n
        case 0xE8:
            ADD16_SP_n(pcPopU8());
            break;
        //INC nn
        case 0x03: case 0x13: case 0x23: case 0x33:
            INC_r(register16Opcodes[opcode>>8]);
            break;
        //DEC nn
        case 0x0B: case 0x1B: case 0x2B: case 0x3B:
            DEC_r(register16Opcodes[opcode>>8]);
            break;
    //Miscellaneous
        case 0x27: //DAA
            DAA();
            break;
        case 0x2F: //CPL
            CPL();
            break;
        case 0x3F: //CCF
            CCF();
            break;
        case 0x37: //SCF
            SCF();
            break;
        case 0x00: //NOP
            NOP();
            break;
        case 0x76: //HALT
            HALT();
            break;
        case 0x10: //STOP
            //TODO: is 0x10 0x00 actually the full op?
            STOP();
            break;
        case 0xF3: //DI
            DI();
            break;
        case 0xFB: //EI
            EI();
            break;
    
    //Rotates and shifts, Bit Operations
        //Many functions: instruction 0xCB changes depending on args
        //Luckily all decode easily
        case 0xCB: {
            uint8_t arg = pcPopU8();
            switch (arg)
            {
            //Rotates and Shifts
            case 0x00 ... 0x07: //RLC n
                RLC_r(registerOpcodes[arg]);
                break;
            case 0x08 ... 0x0F: //RRC n
                RRC_r(registerOpcodes[arg-0x08]);
                break;
            case 0x10 ... 0x17: //RL n
                RL_r(registerOpcodes[arg-0x10]);
                break;
            case 0x18 ... 0x1F: //RR n
                RR_r(registerOpcodes[arg-0x18]);
                break;
            case 0x20 ... 0x27: //SLA n
                SLA_n(registerOpcodes[arg-0x20]);
                break;
            case 0x28 ... 0x2F: //SRA n
                SRA_n(registerOpcodes[arg-0x28]);
                break;
            case 0x30 ... 0x37: //SWAP
                SWAP_n(registerOpcodes[arg-0x30]);
                break;
            case 0x38 ... 0x3F: //SRL n
                SRL_n(registerOpcodes[arg-0x38]);
                break;
            //Bit Operations
            case 0x40 ... 0x47: //BIT b,r
                //TODO: debug width of b
                BIT_b_r(pcPopU8(), registerOpcodes[arg-0x40]);
                break;
            case 0x80 ... 0x87: //RES b,r
                //TODO: debug width of b
                RES_b_r(pcPopU8(), registerOpcodes[arg-0x80]);
                break;
            case 0xC0 ... 0xC7: //SET b,r
                //TODO: debug width of b
                SET_b_r(pcPopU8(), registerOpcodes[arg-0xC0]);
                break;
            default:
                throw("0xCB bad arg");
            }
            break;
        }

    //Jumps
        case 0xC3: //JP nn
            JP_nn(pcPopU16());
            break;
        //JP cc,nn
        case 0xC2: //JP NZ, nn
            JP_cc_nn(Flag::Z, false, pcPopU16());
            break;
        case 0xCA: //JP Z, nn
            JP_cc_nn(Flag::Z, true, pcPopU16());
            break;
        case 0xD2: //JP NC, nn
            JP_cc_nn(Flag::C, false, pcPopU16());
            break;
        case 0xDA: //JP C, nn
            JP_cc_nn(Flag::C, true, pcPopU16());
            break;

        case 0xE9: //JP (HL)
            JP_HL();
            break;
        case 0x18: //JR n
            JR_n(pcPopU8());
            break;
        //JR cc,n
        case 0x20: //JR NZ, n
            JR_cc_n(Flag::Z, false, pcPopU8());
            break;
        case 0x28: //JR Z, n
            JR_cc_n(Flag::Z, true, pcPopU8());
            break;
        case 0x30: //JR NC, n
            JR_cc_n(Flag::C, false, pcPopU8());
            break;
        case 0x38: //JR C, n
            JR_cc_n(Flag::C, true, pcPopU8());
            break;

    //CALLS
        case 0xCD: //CALL nn
            CALL_nn(pcPopU16());
            break;
        //CALL cc, nn
        case 0xC4: //CALL NZ, nn
            CALL_cc_nn(Flag::Z, false, pcPopU16());
            break;
        case 0xCC: //CALL Z, nn
            JR_cc_n(Flag::Z, true, pcPopU16());
            break;
        case 0xD4: //CALL NC, nn
            JR_cc_n(Flag::C, false, pcPopU16());
            break;
        case 0xDC: //CALL C, nn
            JR_cc_n(Flag::C, true, pcPopU16());
            break;

    //Restarts
        //RST n
        case 0xC7: case 0xCF: case 0xD7: case 0xDF: case 0xE7: case 0xEF: case 0xF7: case 0xFF:
            RST_n(opcode-0xC7);
            break;

    //Returns
        case 0xC9: //RET
            RET();
            break;
        //RET cc
        case 0xC0: //RET NZ
            RET_cc(Flag::Z, false);
            break;
        case 0xC8: //RET Z
            RET_cc(Flag::Z, true);
            break;
        case 0xD0: //RET NC
            RET_cc(Flag::C, false);
            break;
        case 0xD8: //RET C
            RET_cc(Flag::C, true);
            break;
        
        case 0xD9: //RETI
            RETI();
            break;
    
    default:
        throw("Bad operation :-(");
        break;
    }

}
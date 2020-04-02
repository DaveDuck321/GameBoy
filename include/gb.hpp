#ifndef gb_hpp
#define gb_hpp

#include <memory>
#include <array>
#include "registers.hpp"
#include "cartridge.hpp"

//http://bgb.bircd.org/pandocs.htm
//https://gekkio.fi/files/gb-docs/gbctr.pdf
//http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
//https://github.com/gbdev/awesome-gbdev#testing

class GB
{
    public:
    Cartridge &cartridge;
    CPURegisters registers;

    std::array<uint8_t, 0x7F> stack;
    std::array<uint8_t, 0x7F> IO;
    std::array<uint8_t, 0x1FFF> workingRam;

    public:
    GB(Cartridge &cartridge);
    ~GB();

    uint8_t pcPopU8(bool debug=true);
    uint16_t pcPopU16();

    uint8_t readU8(uint16_t addr) const;
    uint16_t readU16(uint16_t addr) const;
    void writeU8(uint16_t addr, uint8_t value);
    void writeU16(uint16_t addr, uint16_t value);
    void step();

    //Debug
    void printFlags();

    private:
    // 8-Bit loads
        // LD n, nn
        void LD_n_nn(Register n, uint8_t nn);

        //LD r1, r1
        void LD_r1_r2(Register r1, Register r2);

        //LD n, A
        void LD_r_A(Register r);
        void LD_n_A(uint8_t addr);

        //LD A,(C)
        void LD_A_C();

        //LD A,(C)
        void LD_C_A();

        //LDD A,(HL)
        void LDD_A_HL();

        //LDD (HL),A
        void LDD_HL_A();

        //LDI A,(HL)
        void LDI_A_HL();

        //LDI (HL),A
        void LDI_HL_A();

        //LDH (n),A
        void LDH_n_A(uint8_t n);

        //LDH A,(n)
        void LDH_A_n(uint8_t n);

    //16-Bit loads
        //LD n,nn
        void LD16_n_nn(Register n, uint16_t nn);

        //LD SP,HL
        void LD16_SP_HL();

        //LDHL SP,n
        void LD16_SP_n(int8_t n);

        //LD nn,SP
        void LD_nn_SP(int16_t nn);

        //PUSH nn
        void PUSH(Register r);

        //POP nn
        void POP(Register r);

    //8-Bit ALU
        //ADD n
        void ADD_n(uint8_t n);

        //ADC n
        void ADC_n(uint8_t n);

        //SUB n
        void SUB_n(uint8_t n);

        //SBC n
        void SBC_n(uint8_t n);

        //AND n
        void AND_n(uint8_t n);

        //OR n
        void OR_n(uint8_t n);

        //XOR n
        void XOR_n(uint8_t n);

        //CP n
        void CP_n(uint8_t n);

        //INC n
        void INC_r(Register r);

        //DEC n
        void DEC_r(Register r);

    //16-Bit ALU
        //Helper function for 16-Bit addition
        uint16_t ADD16(uint16_t n1, uint16_t n2);

        //ADD HL,n
        void ADD16_HL_n(Register n);

        //ADD SP,n
        void ADD16_SP_n(uint8_t n);

        //INC nn
        void INC16_nn(Register nn);

        //DEC nn
        void DEC16_nn(Register nn);
    
    //Miscellaneous
        //SWAP n
        void SWAP_n(Register n);

        //DAA
        void DAA();

        //CPL
        void CPL();

        //CCF
        void CCF();

        //SCF
        void SCF();

        //NOP
        void NOP();

        //HALT
        void HALT();

        //STOP
        void STOP();

        //DI
        void DI();

        //EI
        void EI();

    //Rotates and Shifts
        //Helper functions for rotate left
        uint8_t ROT_LC(uint8_t value);
        uint8_t ROT_L(uint8_t value);
        //Helper functions for right rotation
        uint8_t ROT_RC(uint8_t value);
        uint8_t ROT_R(uint8_t value);

        //RLCA
        void RLCA();

        //RLA
        void RLA();

        //RLCA
        void RRCA();

        //RRA
        void RRA();

        //RLC n
        void RLC_r(Register r);

        //RL n
        void RL_r(Register r);

        //RRC n
        void RRC_r(Register r);

        //RR n
        void RR_r(Register r);

        //SLA n
        void SLA_n(Register r);

        //SRA n
        void SRA_n(Register r);

        //SRL n
        void SRL_n(Register r);
    
    //Bit stuff
        //BIT b,r
        void BIT_b_r(uint8_t b, Register r);

        //SET b,r
        void SET_b_r(uint8_t b, Register r);

        //RES b,r
        void RES_b_r(uint8_t b, Register r);
    
    //Jumps
        //JP nn
        void JP_nn(uint16_t nn);

        //JC cc,nn
        void JP_cc_nn(Flag f, bool set, uint16_t nn);

        //JP (HL)
        void JP_HL();

        //JR n
        void JR_n(uint8_t n);

        //JR cc,n
        void JR_cc_n(Flag f, bool set, uint8_t n);
    
    //Calls
        //CALL nn
        void CALL_nn(int16_t nn);

        //CALL cc,nn
        void CALL_cc_nn(Flag f, bool set, uint16_t nn);
    
    //Restarts
        //RST n
        void RST_n(int8_t n);
    
    //Returns
        //RET
        void RET();

        //RET cc
        void RET_cc(Flag f, bool set);

        //RETI
        void RETI();
};

#endif
#pragma once

#include <sys/types.h>
#include "../memory_map.hpp"
#include "../utils/checked_int.hpp"
#include "registers.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace gb {
class Cartridge;
class IO_Manager;

enum class Register : uint8_t {
  A,
  F,
  B,
  C,
  D,
  E,
  H,
  L,
  AF,
  HL,
  BC,
  DE,
  HL_ptr,
  BC_ptr,
  DE_ptr,
  SP,
  PC,
};

class CPU {
  CPURegisters registers;
  CPURegisters comitted_registers;
  MemoryMap* memory_map;
  IO* io;

  // State required for control flow sanitation
  std::optional<uint16_t> current_tos = 0;
  // Since there are no alignment guarantees, this array contains both lower and
  // upper pointers.
  std::vector<uint16_t> return_address_pointers = {};
  std::vector<uint16_t> expected_return_addresses = {};

 public:
  CPU(MemoryMap& memory_map, IO& io);
  CPU(const CPU&) = delete;
  auto operator=(const CPU&) -> CPU& = delete;
  ~CPU() = default;

  auto reset() -> void;

  // Reads cannot be const since they consume 1 cycle
  [[nodiscard]] auto readU8(uint16_t addr, bool allow_undef = false) -> Byte;
  [[nodiscard]] auto readU16(uint16_t addr, bool allow_partial_undef = false)
      -> Word;

  auto writeU8(uint16_t addr, Byte value, bool allow_undef = false) -> void;
  auto writeU16(uint16_t addr, Word value, bool allow_partial_undef = false)
      -> void;

  auto clock() -> void;

  // Debug
  auto getCurrentRegisters() -> CPURegisters&;
  auto getDebugRegisters() -> CPURegisters&;
  auto insertInterruptOnNextCycle(uint8_t id) -> void;

 private:
  [[nodiscard]] auto advancePC1Byte() -> uint8_t;
  [[nodiscard]] auto advancePC2Bytes() -> uint16_t;
  auto handleInterrupts() -> void;
  auto processNextInstruction() -> void;

  // Helper function for the _ptr registers
  auto getRegU8(Register) -> Byte;
  auto setRegU8(Register, Byte) -> void;
  auto getRegU16(Register) -> Word;
  auto setRegU16(Register, Word) -> void;

  // 8-Bit loads
  // LD r, n
  void LD_r_n(Register r, uint8_t n);

  // LD r, (nn)
  void LD_r_nn(Register r, uint16_t nn);

  // LD r1, r1
  void LD_r1_r2(Register r1, Register r2);

  // LD n, A
  void LD_r_A(Register r);
  void LD_nn_A(uint16_t addr);

  // LD A,(C)
  void LD_A_C();

  // LD A,(C)
  void LD_C_A();

  // LDD A,(HL)
  void LDD_A_HL();

  // LDD (HL),A
  void LDD_HL_A();

  // LDI A,(HL)
  void LDI_A_HL();

  // LDI (HL),A
  void LDI_HL_A();

  // LDH (n),A
  void LDH_n_A(uint8_t n);

  // LDH A,(n)
  void LDH_A_n(uint8_t n);

  // 16-Bit loads
  // LD n,nn
  void LD16_n_nn(Register n, uint16_t nn);

  // LD SP,HL
  void LD16_SP_HL();

  // LDHL SP,n
  void LDHL_SP_n(int8_t n);

  // LD nn,SP
  void LD_nn_SP(uint16_t nn);

  // PUSH nn
  void PUSH(Register r);

  // POP nn
  void POP(Register r);

  // 8-Bit ALU
  // ADD n
  void ADD_n(Byte n, bool carry);

  // ADC n
  void ADC_n(Byte n);

  // SUB n
  void SUB_n(Byte n, bool carry);

  // SBC n
  void SBC_n(Byte n);

  // AND n
  void AND_n(Byte n);

  // OR n
  void OR_n(Byte n);

  // XOR n
  void XOR_n(Byte n);

  // CP n
  void CP_n(Byte n);

  // INC n
  void INC_r(Register r);

  // DEC n
  void DEC_r(Register r);

  // 16-Bit ALU
  // Helper function for 16-Bit addition
  auto ADD16(Word n1, Word n2) -> Word;
  auto ADD16_SIGN(Word nn, Byte n, bool derived_from_sp) -> Word;

  // ADD HL,n
  void ADD16_HL_n(Register n);

  // ADD SP,n
  void ADD16_SP_n(int8_t n);

  // INC nn
  void INC16_nn(Register nn);

  // DEC nn
  void DEC16_nn(Register nn);

  // Miscellaneous
  // SWAP n
  void SWAP_n(Register n);

  // DAA
  void DAA();

  // CPL
  void CPL();

  // CCF
  void CCF();

  // SCF
  void SCF();

  // NOP
  void NOP();

  // HALT
  void HALT();

  // STOP
  void STOP();

  // DI
  void DI();

  // EI
  void EI();

  // Rotates and Shifts
  // Helper functions for rotate left
  auto ROT_LC(Byte value) -> Byte;
  auto ROT_L(Byte value) -> Byte;
  // Helper functions for right rotation
  auto ROT_RC(Byte value) -> Byte;
  auto ROT_R(Byte value) -> Byte;

  // RLC n
  void RLCA();             // RLCA
  void RLC_r(Register r);  // RLC r

  // RL n
  void RLA();             // RLA
  void RL_r(Register r);  // RL r

  // RRC n
  void RRCA();             // RRCA
  void RRC_r(Register r);  // RRC r

  // RR n
  void RRA();             // RRA
  void RR_r(Register r);  // RR r

  // SLA n
  void SLA_n(Register r);

  // SRA n
  void SRA_n(Register r);

  // SRL n
  void SRL_n(Register r);

  // Bit stuff
  // BIT b,r
  void BIT_b_r(uint8_t b, Register r);

  // SET b,r
  void SET_b_r(uint8_t b, Register r);

  // RES b,r
  void RES_b_r(uint8_t b, Register r);

  // Jumps
  // JP nn
  void JP_nn(uint16_t nn);

  // JC cc,nn
  void JP_cc_nn(Flag f, bool set, uint16_t nn);

  // JP (HL)
  void JP_HL();

  // JR n
  void JR_n(int8_t n);

  // JR cc,n
  void JR_cc_n(Flag f, bool set, int8_t n);

  // Calls
  // CALL nn
  void CALL_nn(uint16_t nn);

  // CALL cc,nn
  void CALL_cc_nn(Flag f, bool set, uint16_t nn);

  // Restarts
  // RST n
  void RST_n(uint8_t n);

  // Returns
  // RET
  void RET();

  // RET cc
  void RET_cc(Flag f, bool set);

  // RETI
  void RETI();
};

}  // namespace gb

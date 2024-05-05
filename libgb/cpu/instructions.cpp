#include "../io/io.hpp"
#include "cpu.hpp"
#include "registers.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

using namespace gb;

auto CPU::getRegU8(Register reg) -> uint8_t {
  switch (reg) {
    case Register::A:
      return registers.getU8(Reg8::A);
    case Register::F:
      return registers.getU8(Reg8::F);
    case Register::B:
      return registers.getU8(Reg8::B);
    case Register::C:
      return registers.getU8(Reg8::C);
    case Register::D:
      return registers.getU8(Reg8::D);
    case Register::E:
      return registers.getU8(Reg8::E);
    case Register::H:
      return registers.getU8(Reg8::H);
    case Register::L:
      return registers.getU8(Reg8::L);
    case Register::HL_ptr:
      return readU8(registers.getU16(Reg16::HL));
    case Register::BC_ptr:
      return readU8(registers.getU16(Reg16::BC));
    case Register::DE_ptr:
      return readU8(registers.getU16(Reg16::DE));
    default:
      throw std::runtime_error("Register cannot be converted to u8");
  }
}

auto CPU::setRegU8(Register reg, uint8_t value) -> void {
  switch (reg) {
    case Register::A:
      return registers.setU8(Reg8::A, value);
    case Register::F:
      return registers.setU8(Reg8::F, value);
    case Register::B:
      return registers.setU8(Reg8::B, value);
    case Register::C:
      return registers.setU8(Reg8::C, value);
    case Register::D:
      return registers.setU8(Reg8::D, value);
    case Register::E:
      return registers.setU8(Reg8::E, value);
    case Register::H:
      return registers.setU8(Reg8::H, value);
    case Register::L:
      return registers.setU8(Reg8::L, value);
    case Register::HL_ptr:
      return writeU8(registers.getU16(Reg16::HL), value);
    case Register::BC_ptr:
      return writeU8(registers.getU16(Reg16::BC), value);
    case Register::DE_ptr:
      return writeU8(registers.getU16(Reg16::DE), value);
    default:
      throw std::runtime_error("Register cannot be converted to u8");
  }
}

auto CPU::getRegU16(Register reg) -> uint16_t {
  switch (reg) {
    case Register::AF:
      return registers.getU16(Reg16::AF);
    case Register::HL:
      return registers.getU16(Reg16::HL);
    case Register::BC:
      return registers.getU16(Reg16::BC);
    case Register::DE:
      return registers.getU16(Reg16::DE);
    case Register::SP:
      return registers.getU16(Reg16::SP);
    case Register::PC:
      return registers.getU16(Reg16::PC);
    default:
      throw std::runtime_error("Register cannot be converted to u16");
  }
}

auto CPU::setRegU16(Register reg, uint16_t value) -> void {
  switch (reg) {
    case Register::AF:
      return registers.setU16(Reg16::AF, value);
    case Register::HL:
      return registers.setU16(Reg16::HL, value);
    case Register::BC:
      return registers.setU16(Reg16::BC, value);
    case Register::DE:
      return registers.setU16(Reg16::DE, value);
    case Register::SP:
      return registers.setU16(Reg16::SP, value);
    case Register::PC:
      return registers.setU16(Reg16::PC, value);
    default:
      throw std::runtime_error("Register cannot be converted to u16");
  }
}

void CPU::LD_r_n(Register r, uint8_t n) {
  /*
  Description:
      Put value n into r.
  Use with:
      n = B,C,D,E,H,L
      nn = 8 bit immediate value
  */
  setRegU8(r, n);
}

void CPU::LD_r_nn(Register r, uint16_t nn) {
  /*
  Description:
      Put value nn into r.
  Use with:
      r = A,B,C,D,E,H,L,(HL)
      nn = 16 bit immediate value
  */
  setRegU8(r, readU8(nn));
}

void CPU::LD_r1_r2(Register r1, Register r2) {
  /*
  Description:
      Put value r2 into r1.
  Use with:
      r1 = A,B,C,D,E,H,L,(HL)
      r2 = A,B,C,D,E,H,L,(HL),n
  */
  setRegU8(r1, getRegU8(r2));
}

void CPU::LD_r_A(Register r) {
  /*
  Description:
      Put value A into n (registers).
  Use with:
      n = A,B,C,D,E,H,L
  */
  setRegU8(r, registers.r8.a);
}
void CPU::LD_nn_A(uint16_t addr) {
  /*
  Description:
      Put value A into addr (memory).
  Use with:
      n = (BC),(DE),(HL),(nn)
      nn = two byte immediate value. (LS byte first.)
  */
  writeU8(addr, registers.r8.a);
}

void CPU::LD_A_C() {
  /*
  Description:
      Put value at address $FF00 + register C into A.
  Same as:
      LD A,($FF00+C)
  */
  registers.r8.a = readU8(0xFF00U | registers.r8.c);
}

void CPU::LD_C_A() {
  /*
  Description:
      Put A into address $FF00 + register C.
  */
  writeU8(0xFF00U | registers.r8.c, registers.r8.a);
}

void CPU::LDD_A_HL() {
  /*
  Description:
      Put value at address HL into A. Decrement HL.
  Same as:
      LD A,(HL) - DEC HL
  */
  uint16_t hl = getRegU16(Register::HL);
  registers.r8.a = readU8(hl);
  // Don't use DEC16 here. Its free
  setRegU16(Register::HL, hl - 1);
}

void CPU::LDD_HL_A() {
  /*
  Description:
      Put A into memory address HL. Decrement HL.
  Same as:
      LD (HL),A - DEC HL
  */
  uint16_t hl = getRegU16(Register::HL);
  writeU8(hl, registers.r8.a);
  // Don't use DEC16 here. Its free
  setRegU16(Register::HL, hl - 1);
}

void CPU::LDI_A_HL() {
  /*
  Description:
      Put value at address HL into A. Increment HL.
  Same as:
      LD A,(HL) - INC HL
  */
  uint16_t hl = getRegU16(Register::HL);
  registers.r8.a = readU8(hl);
  // Don't use INC16 here. Its free
  setRegU16(Register::HL, hl + 1);
}

void CPU::LDI_HL_A() {
  /*
  Description:
      Put A into memory address HL. Decrement HL.
  Same as:
      LD (HL),A - INC HL
  */
  uint16_t hl = getRegU16(Register::HL);
  writeU8(hl, registers.r8.a);
  // Don't use INC16 here. Its free
  setRegU16(Register::HL, hl + 1);
}

void CPU::LDH_n_A(uint8_t n) {
  /*
  Description:
      Put A into memory address $FF00+n.
  Use with:
      n = one byte immediate value
  */
  writeU8(0xFF00U | n, registers.r8.a);
}

void CPU::LDH_A_n(uint8_t n) {
  /*
  Description:
      Put memory address $FF00+n into A.
  Use with:
      n = one byte immediate value
  */
  registers.r8.a = readU8(0xFF00U | n);
}

void CPU::LD16_n_nn(Register n, uint16_t nn) {
  /*
  Description:
      Put value nn into n.
  Use with:
      n = BC,DE,HL,SP  nn = 16 bit immediate value
  */
  setRegU16(n, nn);
}

void CPU::LD16_SP_HL() {
  /*
  Description:
      Put HL into Stack Pointer (SP)
  */
  io->cycle++;  // 16-Bit load takes an extra cycle
  registers.r16.sp = getRegU16(Register::HL);
}

void CPU::LDHL_SP_n(int8_t n) {
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
  setRegU16(Register::HL, ADD16_SIGN(registers.r16.sp, n));
}

void CPU::LD_nn_SP(uint16_t nn) {
  /*
  Description:
      Put Stack Pointer (SP) at address n.
  Use with:
      nn = two byte immediate address
  */
  writeU16(nn, registers.r16.sp);
}

void CPU::PUSH(Register r) {
  /*
  Description:
      Push register pair nn onto stack.
      Decrement Stack Pointer (SP) twice.
  Use with:
      nn = AF,BC,DE,HL
  */
  // TODO: check order
  io->cycle++;  // PUSH takes extra cycle -- 16-Bit read?
  registers.r16.sp -= 2;
  writeU16(registers.r16.sp, getRegU16(r));
}

void CPU::POP(Register r) {
  /*
  Description:
      Pop two bytes off stack into register pair nn.
      Increment Stack Pointer (SP) twice.
  Use with:
      nn = AF,BC,DE,HL
  */
  // TODO: check order
  setRegU16(r, readU16(registers.r16.sp));
  registers.r16.sp += 2;
}

void CPU::ADD_n(uint8_t n, bool carry) {
  // TODO: guessed C flag operation, might be wrong
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
  const auto carry_int = uint8_t(carry);
  registers.setFlags(Flag::C, (n + carry_int) > 0xFFU - registers.r8.a);
  registers.setFlags(
      Flag::H, ((n & 0x0FU) + carry_int) > (0x0FU - (registers.r8.a & 0x0FU)));

  registers.r8.a = registers.r8.a + n + carry_int;
  registers.setFlags(Flag::Z, registers.r8.a == 0);
  registers.resetFlags(Flag::N);
}

void CPU::ADC_n(uint8_t n) {
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
  ADD_n(n, registers.getFlags(Flag::C));
}

void CPU::SUB_n(uint8_t n, bool carry) {
  // TODO: guessed C flag operation, might be wrong
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
  const uint8_t adjusted_n = n + (uint8_t)carry;
  const bool does_carry_cause_overflow = (n == 0xFF && carry);

  registers.setFlags(Flag::C,
                     registers.r8.a < adjusted_n || does_carry_cause_overflow);
  registers.setFlags(Flag::H,
                     (registers.r8.a & 0x0FU) < (n & 0x0FU) + (uint8_t)carry);

  registers.r8.a = registers.r8.a - adjusted_n;
  registers.setFlags(Flag::Z, registers.r8.a == 0);
  registers.setFlags(Flag::N);
}

void CPU::SBC_n(uint8_t n) {
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
  SUB_n(n, registers.getFlags(Flag::C));
}

void CPU::AND_n(uint8_t n) {
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
  registers.r8.a = registers.r8.a & n;
  registers.setFlags(Flag::Z, registers.r8.a == 0);
  registers.setFlags(Flag::H);
  registers.resetFlags(Flag::N | Flag::C);
}

void CPU::OR_n(uint8_t n) {
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
  registers.r8.a = registers.r8.a | n;
  registers.setFlags(Flag::Z, registers.r8.a == 0);
  registers.resetFlags(Flag::H | Flag::N | Flag::C);
}

void CPU::XOR_n(uint8_t n) {
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
  registers.r8.a = registers.r8.a ^ n;
  registers.setFlags(Flag::Z, registers.r8.a == 0);
  registers.resetFlags(Flag::H | Flag::N | Flag::C);
}

void CPU::CP_n(uint8_t n) {
  /*
  Description:
      Compare A with n. This is basically an A - n subtraction instruction but
  the results are thrown  away.
  Use with:
      n = A,B,C,D,E,H,L,(HL),#
  Flags affected:
    Z - Set if result is zero. (Set if A = n.)
    N - Set.
    H - Set if no borrow from bit 4.
    C - Set for no borrow. (Set if A < n.)
  */
  registers.setFlags(Flag::Z, registers.r8.a == n);
  registers.setFlags(Flag::N);
  registers.setFlags(Flag::H, (registers.r8.a & 0x0FU) < (n & 0x0FU));
  registers.setFlags(Flag::C, registers.r8.a < n);
}

void CPU::INC_r(Register r) {
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
  uint8_t result = getRegU8(r) + 1;
  setRegU8(r, result);
  registers.setFlags(Flag::Z, result == 0);
  registers.setFlags(Flag::H, (result & 0x0FU) == 0);
  registers.resetFlags(Flag::N);
}

void CPU::DEC_r(Register r) {
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
  // TODO: Set if no borrow from bit 4.
  uint8_t result = getRegU8(r) - 1;
  setRegU8(r, result);
  registers.setFlags(Flag::Z, result == 0);
  registers.setFlags(Flag::H, (result & 0x0FU) == 0x0FU);
  registers.setFlags(Flag::N);
}

auto CPU::ADD16(uint16_t n1, uint16_t n2) -> uint16_t {
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
  io->cycle++;  // 16-Bit maths takes an extra cycle
  registers.resetFlags(Flag::N);
  registers.setFlags(Flag::H, (n2 & 0x0FFFU) > (0x0FFFU - (n1 & 0x0FFFU)));
  registers.setFlags(Flag::C, n2 > (0xFFFFU - n1));
  return n1 + n2;
}

auto CPU::ADD16_SIGN(uint16_t nn, int8_t n) -> uint16_t {
  /*
  Description:
      Add n to nn.
  Use with:
      n = one byte signed immediate value (#).
  Flags affected:
      Z - Reset.
      N - Reset.
      H - Set or reset according to operation.
      C - Set or reset according to operation.
  */
  io->cycle++;  // 16-Bit add takes an extra cycle

  const auto usign_n = uint8_t(n);
  registers.setFlags(Flag::H,
                     (usign_n & 0x0FU) > (0x0FU - (registers.r16.sp & 0x0FU)));
  registers.setFlags(Flag::C,
                     (usign_n & 0xFFU) > (0xFFU - (registers.r16.sp & 0xFFU)));
  registers.resetFlags(Flag::N | Flag::Z);
  return nn + n;
}

void CPU::ADD16_HL_n(Register n) {
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
  uint16_t hl_val = getRegU16(Register::HL);
  uint16_t n_val = getRegU16(n);
  setRegU16(Register::HL, ADD16(hl_val, n_val));
}

void CPU::ADD16_SP_n(int8_t n) {
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
  io->cycle++;  // Takes 1 additional cycles
  registers.r16.sp = ADD16_SIGN(registers.r16.sp, n);
}

void CPU::INC16_nn(Register nn) {
  /*
  Description:
      Increment register nn.
  Use with:
      nn = BC,DE,HL,SP
  Flags affected:
      None
  */
  io->cycle++;
  setRegU16(nn, getRegU16(nn) + 1);
}

void CPU::DEC16_nn(Register nn) {
  /*
  Description:
      Decrement register nn.
  Use with:
      nn = BC,DE,HL,SP
  Flags affected:
      None
  */
  io->cycle++;
  setRegU16(nn, getRegU16(nn) - 1);
}

void CPU::SWAP_n(Register n) {
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
  uint8_t value = getRegU8(n);
  registers.setFlags(Flag::Z, value == 0);
  registers.resetFlags(Flag::N | Flag::H | Flag::C);
  setRegU8(n, ((value & 0x0FU) << 4U) | ((value & 0xF0U) >> 4U));
}

void CPU::DAA() {
  /*
  Description:
      Decimal adjust register A.
      This instruction adjusts register A so that the correct representation of
  Binary Coded Decimal (BCD)  is obtained. Flags affected: Z - Set if register A
  is zero. N - Not affected. H - Reset. C - Set or reset according to operation.
  */
  // Adjust differently depending on previous operation
  if (!registers.getFlags(Flag::N)) {
    // When operation was addition, adjust within range
    bool upperAdjust = (registers.r8.a > 0x99U) || registers.getFlags(Flag::C);
    bool lowerAdjust =
        ((registers.r8.a & 0x0FU) > 0x09) || registers.getFlags(Flag::H);

    registers.setFlags(Flag::C, upperAdjust);
    registers.r8.a +=
        (0x60U * uint8_t(upperAdjust)) | (0x06U * uint8_t(lowerAdjust));
  } else {
    // When operation was subtraction, only act on flags
    bool upperAdjust = registers.getFlags(Flag::C);
    bool lowerAdjust = registers.getFlags(Flag::H);

    registers.setFlags(Flag::C, upperAdjust);
    registers.r8.a -=
        (0x60U * uint8_t(upperAdjust)) | (0x06U * uint8_t(lowerAdjust));
  }
  registers.setFlags(Flag::Z, registers.r8.a == 0);
  registers.resetFlags(Flag::H);
}

void CPU::CPL() {
  /*
  Description:
      Complement A register. (Flip all bits.)
  Flags affected:
      Z - Not affected.
      N - Set.
      H - Set.
      C - Not affected.
  */
  registers.setFlags(Flag::N | Flag::H);
  registers.r8.a = ~registers.r8.a;
}

void CPU::CCF() {
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
  registers.resetFlags(Flag::N | Flag::H);
  registers.setFlags(Flag::C, !registers.getFlags(Flag::C));
}

void CPU::SCF() {
  /*
  Description:
      Set Carry flag.
  Flags affected:
      Z - Not affected.
      N - Reset.
      H - Reset.
      C - Set.
  */
  registers.resetFlags(Flag::N | Flag::H);
  registers.setFlags(Flag::C);
}

void CPU::NOP() {
  /*
  Description:
      No operation
  */
}

void CPU::HALT() {
  /*
  Description:
      Power down CPU until an interrupt occurs.
      Use this when ever possible to reduce energy consumption.
  */
  registers.halt = true;
}

void CPU::STOP() {
  /*
  Description:
      Halt CPU & LCD display until button pressed
  */
}

void CPU::DI() {
  /*
  Description:
      This instruction disables interrupts but not immediately.
      Interrupts are disabled after instruction after DI is executed.
  Flags affected:
      None.
  */
  registers.IME[2] = false;
}

void CPU::EI() {
  /*
  Description:
      Enable interrupts. This instruction enables interrupts but not immediately.
      Interrupts are enabled after instruction after EI is executed.
  Flags affected:
      None.
  */
  registers.IME[2] = true;
}

auto CPU::ROT_LC(uint8_t value) -> uint8_t {
  /*
  Description:
      Rotate value left. Old bit 7 to Carry flag.
  Flags affected:
      Z - Unchanged
      N - Reset.
      H - Reset.
      C - Contains old bit 7 data.
  */
  uint8_t result = (value << 1U) | (value >> 7U);

  registers.setFlags(Flag::C, value & 0x80);
  registers.resetFlags(Flag::N | Flag::H);

  return result;
}

auto CPU::ROT_L(uint8_t value) -> uint8_t {
  /*
  Description:
      Rotate value left through Carry flag.
  Flags affected:
      Z - Unchanged
      N - Reset.
      H - Reset.
      C - Contains old bit 7 data.
  */
  uint8_t result = (value << 1U) | uint8_t(registers.getFlags(Flag::C));

  registers.setFlags(Flag::C, bool(value & 0x80U));
  registers.resetFlags(Flag::N | Flag::H);

  return result;
}

auto CPU::ROT_RC(uint8_t value) -> uint8_t {
  /*
  Description:
      Rotate value right. Old bit 0 to Carry flag.
  Flags affected:
      Z - Unchanged
      N - Reset.
      H - Reset.
      C - Contains old bit 0 data
  */
  uint8_t result = ((value & 1U) << 7U) | (value >> 1U);

  registers.setFlags(Flag::C, bool(value & 1U));
  registers.resetFlags(Flag::N | Flag::H);

  return result;
}

auto CPU::ROT_R(uint8_t value) -> uint8_t {
  /*
  Description:
      Rotate value right through Carry flag.
  Flags affected:
      Z - Unchanged.
      N - Reset.
      H - Reset.
      C - Contains old bit 0 data
  */
  uint8_t result = (uint8_t(registers.getFlags(Flag::C)) << 7U) | (value >> 1U);

  registers.setFlags(Flag::C, bool(value & 1U));
  registers.resetFlags(Flag::N | Flag::H);

  return result;
}

void CPU::RLCA() {
  /*
  Description:
      Rotate A left. Old bit 7 to Carry flag.
  Flags affected:
      Z - Reset.
      N - Reset.
      H - Reset.
      C - Contains old bit 7 data.
  */
  registers.resetFlags(Flag::Z);
  registers.r8.a = ROT_LC(registers.r8.a);
}

void CPU::RLA() {
  /*
  Description:
      Rotate A left through Carry flag.
  Flags affected:
      Z - Reset.
      N - Reset.
      H - Reset.
      C - Contains old bit 7 data
  */
  registers.resetFlags(Flag::Z);
  registers.r8.a = ROT_L(registers.r8.a);
}

void CPU::RRCA() {
  /*
  Description:
      Rotate A right. Old bit 0 to Carry flag.
  Flags affected:
      Z - Reset.
      N - Reset.
      H - Reset.
      C - Contains old bit 0 data
  */
  registers.resetFlags(Flag::Z);
  registers.r8.a = ROT_RC(registers.r8.a);
}

void CPU::RRA() {
  /*
  Description:
      Rotate A right through Carry flag.
  Flags affected:
      Z - Reset.
      N - Reset.
      H - Reset.
      C - Contains old bit 0 data
  */
  registers.resetFlags(Flag::Z);
  registers.r8.a = ROT_R(registers.r8.a);
}

void CPU::RLC_r(Register r) {
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
  uint8_t result = ROT_LC(getRegU8(r));
  registers.setFlags(Flag::Z, result == 0);
  setRegU8(r, result);
}

void CPU::RL_r(Register r) {
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
  uint8_t result = ROT_L(getRegU8(r));
  registers.setFlags(Flag::Z, result == 0);
  setRegU8(r, result);
}

void CPU::RRC_r(Register r) {
  /*
  Description:
      Rotate n right. Old bit 0 to Carry flag.
  Use with:
      n = A,B,C,D,E,H,L, (HL)
  Flags affected:
      Z - Set if result is zero.
      N - Reset.
      H - Reset.
      C - Contains old bit 0 data.
  */
  uint8_t result = ROT_RC(getRegU8(r));
  registers.setFlags(Flag::Z, result == 0);
  setRegU8(r, result);
}

void CPU::RR_r(Register r) {
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
  uint8_t result = ROT_R(getRegU8(r));
  registers.setFlags(Flag::Z, result == 0);
  setRegU8(r, result);
}

void CPU::SLA_n(Register r) {
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
  uint8_t value = getRegU8(r);
  setRegU8(r, value << 1U);
  registers.resetFlags(Flag::N | Flag::H);
  registers.setFlags(Flag::Z, (value & 0x7fU) == 0);
  registers.setFlags(Flag::C, bool(value & 0x80U));
}

void CPU::SRA_n(Register r) {
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
  uint8_t value = getRegU8(r);
  uint8_t result = (value >> 1U) | (value & 0x80U);
  registers.resetFlags(Flag::N | Flag::H);
  registers.setFlags(Flag::Z, result == 0);
  registers.setFlags(Flag::C, bool(value & 1U));
  setRegU8(r, result);
}

void CPU::SRL_n(Register r) {
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
  uint8_t value = getRegU8(r);
  uint8_t result = value >> 1U;
  registers.resetFlags(Flag::N | Flag::H);
  registers.setFlags(Flag::Z, result == 0);
  registers.setFlags(Flag::C, bool(value & 1U));
  setRegU8(r, result);
}

void CPU::BIT_b_r(uint8_t b, Register r) {
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
  registers.setFlags(Flag::Z, (getRegU8(r) & (1U << b)) == 0);
  registers.setFlags(Flag::H);
  registers.resetFlags(Flag::N);
}

void CPU::SET_b_r(uint8_t b, Register r) {
  /*
  Description:
      Set bit b in register r.
  Use with:
      b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
  Flags affected:
      None
  */
  setRegU8(r, getRegU8(r) | (1U << b));
}

void CPU::RES_b_r(uint8_t b, Register r) {
  /*
  Description:
      Reset bit b in register r.
  Use with:
      b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
  Flags affected:
      None
  */
  setRegU8(r, getRegU8(r) & ~(1U << b));
}

void CPU::JP_nn(uint16_t nn) {
  /*
  Description:
      Jump to address nn.
  Use with:
      nn = two byte immediate value. (LS byte first.)
  */
  io->cycle++;  // Jumping takes 1 cycle
  registers.r16.pc = nn;
}

void CPU::JP_cc_nn(Flag f, bool set, uint16_t nn) {
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
  if (registers.getFlags(f) == set) {
    JP_nn(nn);
  }
}

void CPU::JP_HL() {
  /*
  Description:
      Jump to address contained in HL
  */
  // Dont use JP function here since HL jump is free (0 cycles)
  registers.r16.pc = getRegU16(Register::HL);
}

void CPU::JR_n(int8_t n) {
  /*
  Description:
      Add n to current address and jump to it.
  Use with:
      n = one byte signed immediate value
  */
  JP_nn(registers.r16.pc + n);
}

void CPU::JR_cc_n(Flag f, bool set, int8_t n) {
  /*
  Description:
      If following condition is true then add n to current address and jump to
  it: Use with: n = one byte signed immediate value cc = NZ, Jump if Z flag is
  reset. cc = Z,  Jump if Z flag is set. cc = NC, Jump if C flag is reset. cc =
  C,  Jump if C flag is set.
  */
  JP_cc_nn(f, set, registers.r16.pc + n);
}

void CPU::CALL_nn(uint16_t nn) {
  /*
  Description:
      Push address of next instruction onto stack and then  jump to address nn.
  Use with:
      nn = two byte immediate value. (LS byte first.)
  */
  PUSH(Register::PC);
  // Don't call JP_nn, the jump should take 0 cycles
  registers.r16.pc = nn;
}

void CPU::CALL_cc_nn(Flag f, bool set, uint16_t nn) {
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
  if (registers.getFlags(f) == set) {
    CALL_nn(nn);
  }
}

void CPU::RST_n(uint8_t n) {
  /*
  Description:
      Push present address onto stack.
      Jump to address $0000 + n.
  Use with:
      n = $00,$08,$10,$18,$20,$28,$30,$38
  */
  PUSH(Register::PC);
  // Don't call JP_nn, the jump should take 0 cycles
  registers.r16.pc = n;
}

void CPU::RET() {
  /*
  Description:
      Pop two bytes from stack & jump to that address
  */
  uint16_t addr = readU16(registers.r16.sp);
  registers.r16.sp += 2;
  JP_nn(addr);
}

void CPU::RET_cc(Flag f, bool set) {
  /*
  Description:
      Return if following condition is true:
  Use with:
      cc = NZ, Return if Z flag is reset.
      cc = Z,  Return if Z flag is set.
      cc = NC, Return if C flag is reset.
      cc = C,  Return if C flag is set.
  */
  io->cycle++;  // This takes longer for some reason
  if (registers.getFlags(f) == set) {
    RET();
  }
}

void CPU::RETI() {
  /*
  Description:
      Pop two bytes from stack & jump to that address then enable interrupts
  */

  // Removed IE() since there is no delay in enabling interrupts
  registers.IME.fill(true);
  RET();
}

#pragma once

#include "../utils/checked_int.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace gb {
// Note register mapping is internal-only... This is intentionally different to
// the instruction encoding.
enum class Reg8 : uint8_t {
  F = 0,
  A = 1,
  C = 2,
  B = 3,
  E = 4,
  D = 5,
  L = 6,
  H = 7,
};

enum class Reg16 : uint16_t {
  AF = 0,
  BC = 1,
  DE = 2,
  HL = 3,
  SP = 4,
  PC = 5,
};

enum class Flag : uint8_t {
  None = 0x00,
  C = 0x10,
  H = 0x20,
  N = 0x40,
  Z = 0x80
};
inline auto operator|(Flag f1, Flag f2) -> Flag {
  return Flag(std::to_underlying(f1) | std::to_underlying(f2));
};
inline auto operator&(Flag f1, Flag f2) -> Flag {
  return Flag(std::to_underlying(f1) & std::to_underlying(f2));
};

// Dont really like this, but it makes everything else prettier
// GB is used for pointer registers
class GB;

struct CPURegisters {
  Byte a{0x01};
  Byte _f{0xb0};
  Byte b{0x00};
  Byte c{0x13};
  Byte d{0x00};
  Byte e{0xd8};
  Byte h{0x01};
  Byte l{0x4d};

  // Plan uint16_t since these MUST never be undef
  uint16_t sp{0xfffe};
  uint16_t pc{0x0100};
  bool halt = false;

  // Need 3 queued IME values for any switching combo
  // Start with interrupts enabled
  std::array<bool, 3> IME = {true, true, true};

  constexpr CPURegisters() = default;

  [[nodiscard]] auto getFlags(Flag mask) const -> bool {
    Byte mask_val{std::to_underlying(mask)};
    return Flag((_f & mask_val).decay()) != Flag::None;
  }

  auto setFlags(Flag mask, bool set = true) -> void {
    Byte mask_val{std::to_underlying(mask)};
    _f = (_f & ~mask_val) | (set ? mask_val : 0_B);
  };
  auto resetFlags(Flag mask) -> void { setFlags(mask, /*set=*/false); }

  [[nodiscard]] auto getU8(Reg8 r) const -> Byte {
    switch (r) {
      case Reg8::F:
        return _f;
      case Reg8::A:
        return a;
      case Reg8::B:
        return b;
      case Reg8::C:
        return c;
      case Reg8::D:
        return d;
      case Reg8::E:
        return e;
      case Reg8::H:
        return h;
      case Reg8::L:
        return l;
    }
    assert(!"Unreachable");
  }

  [[nodiscard]] auto getU16(Reg16 r) const -> Word {
    switch (r) {
      case Reg16::AF:
        return {a, _f};
      case Reg16::BC:
        return {b, c};
      case Reg16::DE:
        return {d, e};
      case Reg16::HL:
        return {h, l};
      case Reg16::SP:
        return Word{sp, {.derived_from_sp = true, .undefined = false}};
      case Reg16::PC:
        return Word{pc};
    }
    assert(!"Unreachable");
  }

  auto setU16(Reg16 r, Word value) -> void {
    switch (r) {
      case Reg16::AF:
        a = value.upper();
        _f = value.lower() & 0xF0_B;
        break;
      case Reg16::BC:
        b = value.upper();
        c = value.lower();
        break;
      case Reg16::DE:
        d = value.upper();
        e = value.lower();
        break;
      case Reg16::HL:
        h = value.upper();
        l = value.lower();
        break;
      case Reg16::SP:
        sp = value.decay();
        break;
      case Reg16::PC:
        pc = value.decay();
        break;
    }
  }

  auto setU8(Reg8 r, Byte value) -> void {
    switch (r) {
      case Reg8::F:
        _f = value & 0xF0_B;
        break;
      case Reg8::A:
        a = value;
        break;
      case Reg8::B:
        b = value;
        break;
      case Reg8::C:
        c = value;
        break;
      case Reg8::D:
        d = value;
        break;
      case Reg8::E:
        e = value;
        break;
      case Reg8::H:
        h = value;
        break;
      case Reg8::L:
        l = value;
        break;
    }
  }
};
}  // namespace gb

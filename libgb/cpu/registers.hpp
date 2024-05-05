#pragma once

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
  union {
    struct {
      uint8_t _f;  // Must not be used directly
      uint8_t a;
      uint8_t c;
      uint8_t b;
      uint8_t e;
      uint8_t d;
      uint8_t l;
      uint8_t h;
    } r8;
    struct {
      uint16_t _af;  // Must not be used directly
      uint16_t bc;
      uint16_t de;
      uint16_t hl;
      uint16_t sp;
      uint16_t pc;
    } r16;
    std::array<uint8_t, 8> r8_view;
    std::array<uint16_t, 6> r16_view;
  };
  bool halt = false;

  // Need 3 queued IME values for any switching combo
  // Start with interrupts enabled
  std::array<bool, 3> IME = {true, true, true};

  explicit CPURegisters()
      : r16{
            ._af = 0x01B0,
            .bc = 0x0013,
            .de = 0x00D8,
            .hl = 0x014D,
            .sp = 0xFFFE,
            .pc = 0x0100,
        } {}

  [[nodiscard]] auto getFlags(Flag mask) const -> bool {
    return (Flag(r8._f) & mask) != Flag::None;
  }

  auto setFlags(Flag mask, bool set = true) -> void {
    uint8_t mask_val = std::to_underlying(mask);
    r8._f = (r8._f & ~mask_val) | (set ? mask_val : 0U);
  };
  auto resetFlags(Flag mask) -> void { setFlags(mask, /*set=*/false); }

  [[nodiscard]] auto getU8(Reg8 r) const -> uint8_t {
    return r8_view[std::to_underlying(r)];
  }
  [[nodiscard]] auto getU16(Reg16 r) const -> uint16_t {
    return r16_view[std::to_underlying(r)];
  }

  auto setU8(Reg8 r, uint8_t value) -> void {
    if (r == Reg8::F) {
      r8_view[std::to_underlying(r)] = value & 0xF0U;
    } else {
      r8_view[std::to_underlying(r)] = value;
    }
  }
  auto setU16(Reg16 r, uint16_t value) -> void {
    if (r == Reg16::AF) {
      r16_view[std::to_underlying(r)] = value & 0xFFF0U;
    } else {
      r16_view[std::to_underlying(r)] = value;
    }
  }
};

}  // namespace gb

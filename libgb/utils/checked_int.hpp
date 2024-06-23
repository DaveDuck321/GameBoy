#pragma once

#include "../error_handling.hpp"

#include <cassert>
#include <compare>
#include <cstdint>
#include <limits>

namespace gb {

template <std::integral Underlying, typename Decorated>
struct CheckedInt {
  struct Flags {
    bool derived_from_sp : 1;
    bool undefined : 1;
  };
  Underlying data;
  Flags flags;

  constexpr CheckedInt()
      : data{0}, flags{.derived_from_sp = false, .undefined = true} {}
  constexpr CheckedInt(Underlying data, Flags flags)
      : data{data}, flags{flags} {}
  explicit constexpr CheckedInt(Underlying data)
      : data{data}, flags{.derived_from_sp = false, .undefined = false} {}

  [[nodiscard]] constexpr auto decay() const -> Underlying {
    if (flags.undefined) {
      throw UndefinedDataError("Attempt to decay undefined byte");
    }
    return data;
  }

  [[nodiscard]] constexpr auto decay_or(Underlying if_undef) const
      -> Underlying {
    return flags.undefined ? if_undef : data;
  }

  [[nodiscard]] constexpr auto operator+(Decorated other) const -> Decorated {
    if (flags.undefined || other.flags.undefined) {
      throw UndefinedDataError("Attempt to add undefined byte");
    }
    bool derived_from_sp = flags.derived_from_sp || other.flags.derived_from_sp;
    return {static_cast<Underlying>(data + other.data),
            Flags{
                .derived_from_sp = derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator-(Decorated other) const -> Decorated {
    if (flags.undefined || other.flags.undefined) {
      throw UndefinedDataError("Attempt to sub undefined byte");
    }
    bool derived_from_sp = flags.derived_from_sp || other.flags.derived_from_sp;
    return {static_cast<Underlying>(data - other.data),
            Flags{
                .derived_from_sp = derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator|(Decorated other) const -> Decorated {
    if (flags.undefined || other.flags.undefined) {
      throw UndefinedDataError("Attempt to or undefined byte");
    }
    bool derived_from_sp = flags.derived_from_sp || other.flags.derived_from_sp;
    return {static_cast<Underlying>(data | other.data),
            Flags{
                .derived_from_sp = derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator&(Decorated other) const -> Decorated {
    // Some games use AND 0 to clear a + flags
    bool const is_well_defined =
        (not other.flags.undefined && other.data == 0) ||
        (not flags.undefined && data == 0) ||
        (not other.flags.undefined && not flags.undefined);

    if (not is_well_defined) {
      throw UndefinedDataError("Attempt to and undefined byte");
    }
    bool derived_from_sp = flags.derived_from_sp || other.flags.derived_from_sp;
    return {static_cast<Underlying>(data & other.data),
            Flags{
                .derived_from_sp = derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator^(Decorated other) const -> Decorated {
    if (flags.undefined || other.flags.undefined) {
      throw UndefinedDataError("Attempt to xor undefined byte");
    }
    bool derived_from_sp = flags.derived_from_sp || other.flags.derived_from_sp;
    return {static_cast<Underlying>(data ^ other.data),
            Flags{
                .derived_from_sp = derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator~() const -> Decorated {
    if (flags.undefined) {
      throw UndefinedDataError("Attempt to negate undefined byte");
    }
    return {static_cast<Underlying>(~data),
            Flags{
                .derived_from_sp = flags.derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator>>(size_t amount) const -> Decorated {
    if (flags.undefined) {
      throw UndefinedDataError("Attempt to rshift undefined byte");
    }
    return {static_cast<Underlying>(data >> amount),
            Flags{
                .derived_from_sp = flags.derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator<<(size_t amount) const -> Decorated {
    if (flags.undefined) {
      throw UndefinedDataError("Attempt to lshift undefined byte");
    }
    return {static_cast<Underlying>(data << amount),
            Flags{
                .derived_from_sp = flags.derived_from_sp,
                .undefined = false,
            }};
  }

  [[nodiscard]] constexpr auto operator==(const CheckedInt& other) const
      -> bool {
    return (*this <=> other) == std::strong_ordering::equal;
  }

  [[nodiscard]] constexpr auto operator<=>(const CheckedInt& other) const
      -> std::strong_ordering {
    if (flags.undefined || other.flags.undefined) {
      throw UndefinedDataError("Attempt to compare undefined byte");
    }
    return data <=> other.data;
  }
};

struct Byte : CheckedInt<uint8_t, Byte> {
  using CheckedInt::CheckedInt;
};

// Special case, can be split up into potentially undefined bytes
struct Word : CheckedInt<uint16_t, Word> {
  bool high_undefined = false;
  bool low_undefined = false;

  using CheckedInt::CheckedInt;

  constexpr Word(Byte upper, Byte lower)
      : CheckedInt{(uint16_t)((uint16_t)((upper.data << 8U)) | lower.data),
                   {
                       .derived_from_sp = upper.flags.derived_from_sp ||
                                          lower.flags.derived_from_sp,
                       .undefined =
                           upper.flags.undefined || lower.flags.undefined,
                   }},
        high_undefined{upper.flags.undefined},
        low_undefined(lower.flags.undefined) {}

  constexpr auto lower() -> Byte {
    return Byte{
        static_cast<uint8_t>(data & 0xFFU),
        {.derived_from_sp = flags.derived_from_sp, .undefined = low_undefined}};
  }

  constexpr auto upper() -> Byte {
    return Byte{static_cast<uint8_t>(data >> 8U),
                {.derived_from_sp = flags.derived_from_sp,
                 .undefined = high_undefined}};
  }
};

constexpr auto operator""_B(unsigned long long data) -> Byte {
  assert(data <= std::numeric_limits<uint8_t>::max());
  return Byte{static_cast<uint8_t>(data)};
}

constexpr auto operator""_W(unsigned long long data) -> Word {
  assert(data <= std::numeric_limits<uint16_t>::max());
  return Word{static_cast<uint16_t>(data)};
}

}  // namespace gb

#pragma once

#include <cstdint>

namespace gb::io_registers {
constexpr uint16_t IO_OFFSET = 0xFF00;

constexpr uint16_t LCD_STAT = 0x41;

// Interrupts
constexpr uint16_t INTERRUPTS = 0x0F;
constexpr uint8_t TIMER_INTERRUPT = 0x04;
constexpr uint8_t SERIAL_INTERRUPT = 0x08;
constexpr uint8_t INPUT_INTERRUPT = 0x10;

// Timers
constexpr uint16_t DIV_TIMER = 0x04;
constexpr uint16_t T_COUNTER = 0x05;
constexpr uint16_t T_MODULO = 0x06;
constexpr uint16_t T_CONTROL = 0x07;

// Serial
constexpr uint16_t SERIAL_DATA = 0x01;
constexpr uint16_t SERIAL_CTL = 0x02;

// Audio
constexpr uint16_t FIRST_APU_REGISTER = 0x10;
constexpr uint16_t LAST_APU_REGISTER = 0x26;

constexpr uint16_t CHANNEL1_SWEEP = 0x10;            // NR10
constexpr uint16_t CHANNEL1_LENGTH_DUTY = 0x11;      // NR11
constexpr uint16_t CHANNEL1_VOLUME_ENVELOPE = 0x12;  // NR12
constexpr uint16_t CHANNEL1_PERIOD_LOW = 0x13;       // NR13
constexpr uint16_t CHANNEL1_CTL = 0x14;              // NR14

constexpr uint16_t CHANNEL2_LENGTH_DUTY = 0x16;      // NR21
constexpr uint16_t CHANNEL2_VOLUME_ENVELOPE = 0x17;  // NR22
constexpr uint16_t CHANNEL2_PERIOD_LOW = 0x18;       // NR23
constexpr uint16_t CHANNEL2_CTL = 0x19;              // NR24

constexpr uint16_t CHANNEL4_LENGTH_TIMER = 0x20;     // NR41
constexpr uint16_t CHANNEL4_VOLUME_ENVELOPE = 0x21;  // NR42
constexpr uint16_t CHANNEL4_RANDOMNESS = 0x22;       // NR43
constexpr uint16_t CHANNEL4_CTL = 0x23;              // NR44

constexpr uint16_t CHANNEL3_DAC = 0x1A;           // NR30
constexpr uint16_t CHANNEL3_LENGTH_TIMER = 0x1B;  // NR31
constexpr uint16_t CHANNEL3_VOLUME = 0x1C;        // NR32
constexpr uint16_t CHANNEL3_PERIOD_LOW = 0x1D;    // NR33
constexpr uint16_t CHANNEL3_CTL = 0x1E;           // NR34

struct BasicChannelRegisters {
  uint16_t length;
  uint16_t volume_envelope;
  uint16_t ctl;
  uint16_t frequency;
};

static constexpr auto Channel1Registers = BasicChannelRegisters{
    .length = CHANNEL1_LENGTH_DUTY,
    .volume_envelope = CHANNEL1_VOLUME_ENVELOPE,
    .ctl = CHANNEL1_CTL,
    .frequency = CHANNEL1_PERIOD_LOW,
};

static constexpr auto Channel2Registers = BasicChannelRegisters{
    .length = CHANNEL2_LENGTH_DUTY,
    .volume_envelope = CHANNEL2_VOLUME_ENVELOPE,
    .ctl = CHANNEL2_CTL,
    .frequency = CHANNEL2_PERIOD_LOW,
};

static constexpr auto Channel3Registers = BasicChannelRegisters{
    .length = CHANNEL3_LENGTH_TIMER,
    .volume_envelope = CHANNEL3_VOLUME,
    .ctl = CHANNEL3_CTL,
    .frequency = CHANNEL3_PERIOD_LOW,
};

static constexpr auto Channel4Registers = BasicChannelRegisters{
    .length = CHANNEL4_LENGTH_TIMER,
    .volume_envelope = CHANNEL4_VOLUME_ENVELOPE,
    .ctl = CHANNEL4_CTL,
    .frequency = CHANNEL4_RANDOMNESS,
};

constexpr uint16_t WAVE_PATTERN_START = 0x30;
constexpr uint16_t WAVE_PATTERN_LAST = 0x3F;

constexpr uint16_t MASTER_VOLUME = 0x24;     // NR50
constexpr uint16_t SOUND_PANNING = 0x25;     // NR51
constexpr uint16_t AUDIO_MASTER_CTL = 0x26;  // NR52
}  // namespace gb::io_registers

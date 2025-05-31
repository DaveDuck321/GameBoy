#pragma once

#include "io_registers.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gb {

static constexpr size_t AUDIO_SAMPLE_FREQUENCY = 1UL << 22UL;
static constexpr size_t APPROX_PLAYBACK_FREQUENCY = 4213440UL;

class APU {
  std::span<uint8_t, 0x80> io_memory;

  size_t m_gameboy_clocks_per_host_sample;
  size_t m_clocks_till_sample = 0;

  size_t m_last_clock = 0;
  size_t m_div_apu_counter = 0;
  size_t m_last_div_value = 0;

  bool m_apu_has_power = false;

  struct BasicChannel {
    io_registers::BasicChannelRegisters regs{};
    size_t length_timer = 0;
    size_t peek_level = 0;
    size_t envelope_sweep_pace = 0;
    size_t envelope_timer = 0;
    size_t sample_point = 0;
    size_t counter = 0;
    uint8_t current_output_level = 0;
    bool envelope_increases = false;
    bool channel_on = false;
  };

  BasicChannel m_channel1;
  BasicChannel m_channel2;
  BasicChannel m_channel3;
  BasicChannel m_channel4;
  size_t m_channel1_sweep_countdown = 0;
  bool m_channel1_sweep_active = false;
  bool m_channel1_sweep_locked_until_trigger = false;

  uint16_t m_channel4_lsr = 0;

  std::vector<std::pair<float, float>> m_samples_since_last_flush;

  // Remove the DC offset
  size_t m_number_of_samples = 0;
  double m_sum_of_samples_l = 0.0;
  double m_sum_of_samples_r = 0.0;

 public:
  explicit APU(std::span<uint8_t, 0x80> io_memory, size_t host_sample_frequency)
      : io_memory{io_memory},
        m_gameboy_clocks_per_host_sample{APPROX_PLAYBACK_FREQUENCY /
                                         host_sample_frequency} {
    m_channel1.regs = io_registers::Channel1Registers;
    m_channel2.regs = io_registers::Channel2Registers;
    m_channel3.regs = io_registers::Channel3Registers;
    m_channel4.regs = io_registers::Channel4Registers;
  }
  auto write(uint16_t addr, uint8_t value) -> void;
  auto read(uint16_t addr) -> uint8_t;

  auto get_samples() -> std::span<std::pair<float, float>>;
  auto flush_samples(size_t count) -> void;

  auto clock_to(size_t target_clock) -> void;

  auto tick_div_apu() -> void;
  auto tick_audio() -> void;
  auto sample_audio() -> void;

  auto tick_pulse_channel(BasicChannel&) -> void;
  auto tick_channel3() -> void;
  auto tick_channel4() -> void;

  auto ch1_freq_sweep_event() -> void;
  auto envelope_sweep_event(BasicChannel&) -> void;

  auto sound_length_event_channel3() -> void;
  auto sound_length_event(BasicChannel&) -> void;

  auto rearm_channel1_sweep() -> void;
  auto channel1_sweep_calculate_or_overflow() -> size_t;
  auto trigger_channel3() -> void;
  auto trigger_pwm_channel(BasicChannel&) -> void;
  auto rearm_pwm_channel_length(BasicChannel&) -> void;
  auto rearm_channel3_length() -> void;
  auto rearm_pwm_channel_volume_envelope(BasicChannel&) -> void;
  auto rearm_channel3_volume() -> void;

  auto power_up() -> void;
  auto power_down() -> void;
};
}  // namespace gb

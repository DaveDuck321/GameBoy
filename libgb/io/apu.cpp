#include "apu.hpp"
#include "io_registers.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>

using namespace gb;
using namespace gb::io_registers;

namespace {

constexpr std::array<bool, 8> pulse_00 = {true, true, true, true,
                                          true, true, true, false};
constexpr std::array<bool, 8> pulse_01 = {false, true, true, true,
                                          true,  true, true, false};
constexpr std::array<bool, 8> pulse_10 = {false, true,  true,  true,
                                          true,  false, false, false};
constexpr std::array<bool, 8> pulse_11 = {true,  false, false, false,
                                          false, false, false, true};
constexpr std::array pulses = {pulse_00, pulse_01, pulse_10, pulse_11};

auto read_channel_period(std::span<uint8_t, 0x80> io_memory,
                         uint16_t lower_addr,
                         uint16_t upper_addr) -> uint16_t {
  uint16_t result = 0;
  result |= io_memory[lower_addr];
  result |= (uint16_t)((uint16_t)(io_memory[upper_addr] & 0b111U) << 8U);
  return result;
}

auto write_channel_period(std::span<uint8_t, 0x80> io_memory,
                          uint16_t lower_addr,
                          uint16_t upper_addr,
                          uint16_t value) -> void {
  assert(value < (1U << 11U));
  io_memory[lower_addr] = value & 0xFFU;
  io_memory[upper_addr] =
      (io_memory[upper_addr] & 0xF8U) | (uint16_t)(value >> 8U);
}

}  // namespace

auto APU::tick_div_apu() -> void {
  size_t div_timer = io_memory[io_registers::DIV_TIMER];

  const auto last_div_timer_bit4 = (m_last_div_value >> 4U) & 1U;
  const auto current_div_timer_bit4 = (div_timer >> 4U) & 1U;
  bool did_tick_div_apu =
      last_div_timer_bit4 == 1 && current_div_timer_bit4 == 0;

  m_last_div_value = div_timer;

  // https://gbdev.io/pandocs/Audio_details.html#div-apu
  if (did_tick_div_apu) {
    m_div_apu_counter += 1;

    if (m_div_apu_counter % 8 == 0) {
      envelope_sweep_event(m_channel1);
      envelope_sweep_event(m_channel2);
      envelope_sweep_event(m_channel4);
    }

    if (m_div_apu_counter % 2 == 0) {
      sound_length_event(m_channel1);
      sound_length_event(m_channel2);
      sound_length_event(m_channel4);
      sound_length_event_channel3();
    }

    if (m_div_apu_counter % 4 == 0) {
      ch1_freq_sweep_event();
    }
  }
}

auto APU::write(uint16_t addr, uint8_t value) -> void {
  if (addr == IO_OFFSET + AUDIO_MASTER_CTL) {
    if ((value & 0x80U) != 0) {
      power_up();
    } else {
      power_down();
    }
    return;
  }

  if (not m_apu_has_power) {
    // Writes are ignored when the APU is unpowered
    return;
  }

  uint16_t io_reg = addr - IO_OFFSET;

  BasicChannel* channel = nullptr;
  if (io_reg >= CHANNEL1_SWEEP && io_reg <= CHANNEL1_CTL) {
    assert(channel == nullptr);
    channel = &m_channel1;
  }
  if (io_reg >= CHANNEL2_LENGTH_DUTY && io_reg <= CHANNEL2_CTL) {
    assert(channel == nullptr);
    channel = &m_channel2;
  }
  if (io_reg >= CHANNEL3_DAC && io_reg <= CHANNEL4_CTL) {
    assert(channel == nullptr);
    channel = &m_channel4;
  }

  switch (io_reg) {
    case CHANNEL1_SWEEP:
      io_memory[io_reg] = value;

      if (m_channel1_sweep_locked_until_trigger) {
        // An active sweep will only pick up disables, everything else waits
        // until an iteration completes (or the channel is retriggered)
        auto is_disabled = ((uint8_t)(value >> 4U) & 0b111U) == 0;
        if (is_disabled) {
          // Sweep disable is instant
          m_channel1_sweep_active = false;
        }
      } else {
        // An inactive sweep will immediately pick up changes to the config
        // register
        rearm_channel1_sweep();
      }
      break;
    case CHANNEL1_CTL:
    case CHANNEL2_CTL:
    case CHANNEL4_CTL: {
      io_memory[io_reg] = value;

      auto triggered = (value & 0x80U) != 0;
      if (triggered) {
        assert(channel != nullptr);
        trigger_pwm_channel(*channel);
      }
      break;
    }
    case CHANNEL3_CTL: {
      io_memory[io_reg] = value;

      auto triggered = (value & 0x80U) != 0;
      if (triggered) {
        trigger_channel3();
      }
      break;
    }
    case CHANNEL1_LENGTH_DUTY:
    case CHANNEL2_LENGTH_DUTY:
    case CHANNEL4_LENGTH_TIMER:
      io_memory[io_reg] = value;

      assert(channel != nullptr);
      rearm_pwm_channel_length(*channel);
      break;
    case CHANNEL3_LENGTH_TIMER:
      io_memory[io_reg] = value;
      rearm_channel3_length();
      break;
    case CHANNEL1_VOLUME_ENVELOPE:
    case CHANNEL2_VOLUME_ENVELOPE:
    case CHANNEL4_VOLUME_ENVELOPE: {
      io_memory[io_reg] = value;

      // Writing 0s to initial volume + dir disables the DAC immediately
      bool is_dac_enabled = (value >> 3U) != 0;
      if (not is_dac_enabled) {
        assert(channel != nullptr);
        channel->channel_on = false;
      }
      break;
    }
    case CHANNEL3_DAC: {
      io_memory[io_reg] = value;

      // Writing 0s to initial volume + dir disables the DAC immediately
      bool is_dac_enabled = (value & 0x80U) != 0;
      if (not is_dac_enabled) {
        m_channel3.channel_on = false;
      }
      break;
    }
    default:
      // Normally we don't care, just forward to memory
      io_memory[io_reg] = value;
  }
}

auto APU::read(uint16_t addr) -> uint8_t {
  uint16_t io_reg = addr - IO_OFFSET;
  uint16_t unreadable_mask = 0x00;
  switch (io_reg) {
    default:
      // Unassigned register
      unreadable_mask = 0xFF;
      break;
    case AUDIO_MASTER_CTL:
      if (not m_apu_has_power) {
        return 0x70;
      }
      return 0xf0U | (uint8_t)((uint8_t)m_channel4.channel_on << 3U) |
             (uint8_t)((uint8_t)m_channel3.channel_on << 2U) |
             (uint8_t)((uint8_t)m_channel2.channel_on << 1U) |
             (uint8_t)m_channel1.channel_on;
    case SOUND_PANNING:
    case MASTER_VOLUME:
    case CHANNEL1_VOLUME_ENVELOPE:
    case CHANNEL2_VOLUME_ENVELOPE:
    case CHANNEL4_VOLUME_ENVELOPE:
    case CHANNEL4_RANDOMNESS:
    case WAVE_PATTERN_START ... WAVE_PATTERN_LAST:
      break;  // No mask
    case CHANNEL1_LENGTH_DUTY:
    case CHANNEL2_LENGTH_DUTY:
      unreadable_mask = 0b111111;
      break;
    case CHANNEL1_SWEEP:
      unreadable_mask = 0x80;
      break;
    case CHANNEL1_CTL:
    case CHANNEL2_CTL:
    case CHANNEL3_CTL:
    case CHANNEL4_CTL:
      unreadable_mask = 0xFFU ^ (1U << 6U);
      break;
    case CHANNEL3_DAC:
      unreadable_mask = 0x7F;
      break;
    case CHANNEL3_VOLUME:
      unreadable_mask = 0x9F;
      break;
  }
  // Normally we don't care, just forward from memory
  return io_memory[io_reg] | unreadable_mask;
}

auto APU::clock_to(size_t target_clock) -> void {
  if (m_last_clock != target_clock) {
    tick_div_apu();
  }

  while (m_last_clock != target_clock) {
    m_last_clock += 1;
    tick_audio();
    sample_audio();
  }
}

auto APU::tick_audio() -> void {
  if (m_last_clock % 2 == 0) {
    tick_channel3();
  }
  if (m_last_clock % 4 == 0) {
    tick_pulse_channel(m_channel1);
    tick_pulse_channel(m_channel2);
    tick_channel4();
  }
}

auto APU::sample_audio() -> void {
  if (m_clocks_till_sample != 0) {
    m_clocks_till_sample -= 1;
    return;
  }

  m_clocks_till_sample = m_gameboy_clocks_per_host_sample;

  float DAC1_out = 0.0F;
  float DAC2_out = 0.0F;
  float DAC3_out = 0.0F;
  float DAC4_out = 0.0F;
  if (m_channel1.channel_on) {
    DAC1_out = (7.5F - (float)m_channel1.current_output_level) / 7.5F;
  }

  if (m_channel2.channel_on) {
    DAC2_out = (7.5F - (float)m_channel2.current_output_level) / 7.5F;
  }

  if (m_channel3.channel_on) {
    DAC3_out = (7.5F - (float)m_channel3.current_output_level) / 7.5F;
  }

  if (m_channel4.channel_on) {
    DAC4_out = (7.5F - (float)m_channel4.current_output_level) / 7.5F;
  }

  auto panning = io_memory[SOUND_PANNING];
  float left_out = ((panning & (1U << 4U)) != 0 ? DAC1_out : 0.0F) +
                   ((panning & (1U << 5U)) != 0 ? DAC2_out : 0.0F) +
                   ((panning & (1U << 6U)) != 0 ? DAC3_out : 0.0F) +
                   ((panning & (1U << 7U)) != 0 ? DAC4_out : 0.0F);

  float right_out = ((panning & (1U << 0U)) != 0 ? DAC1_out : 0.0F) +
                    ((panning & (1U << 1U)) != 0 ? DAC2_out : 0.0F) +
                    ((panning & (1U << 2U)) != 0 ? DAC3_out : 0.0F) +
                    ((panning & (1U << 3U)) != 0 ? DAC4_out : 0.0F);

  auto volume = io_memory[MASTER_VOLUME];

  float scale_right = 0.0F;
  float scale_left = 0.0F;

  uint8_t left_volume = (uint8_t)(volume >> 4U) & 0b111U;
  scale_left = ((float)(left_volume + 1)) / 8.0F;

  uint8_t right_volume = volume & 0b111U;
  scale_right = ((float)(right_volume + 1)) / 8.0F;

  float left_mixer_output = 0.25F * left_out * scale_left;
  float right_mixer_output = 0.25F * right_out * scale_right;

  m_number_of_samples += 1;
  m_sum_of_samples_l += left_mixer_output;
  m_sum_of_samples_r += right_mixer_output;

  // Save stereo audio normalized between -1 and 1
  m_samples_since_last_flush.emplace_back(left_mixer_output,
                                          right_mixer_output);
}

auto APU::tick_pulse_channel(APU::BasicChannel& channel) -> void {
  if (not channel.channel_on) {
    return;
  }

  channel.counter += 1;
  if (channel.counter == 0x800) {
    // Counter overflow, change sample point
    channel.counter = read_channel_period(io_memory, channel.regs.frequency,
                                          channel.regs.ctl);

    channel.sample_point = (channel.sample_point + 1) % 8;

    // Calculate level
    size_t current_duty = io_memory[channel.regs.length] >> 6U;
    assert(current_duty < 4);
    if (pulses[current_duty][channel.sample_point]) {
      channel.current_output_level = channel.peek_level;
    } else {
      channel.current_output_level = 0;
    }
  }
}

auto APU::tick_channel3() -> void {
  if (not m_channel3.channel_on) {
    return;
  }
  static constexpr size_t wave_pattern_length = 32;
  static_assert(2UL * (WAVE_PATTERN_LAST - WAVE_PATTERN_START + 1) ==
                wave_pattern_length);

  m_channel3.counter += 1;
  if (m_channel3.counter == 0x800) {
    // Counter overflow, change sample point
    m_channel3.counter = read_channel_period(
        io_memory, m_channel3.regs.frequency, m_channel3.regs.ctl);

    m_channel3.sample_point =
        (m_channel3.sample_point + 1) % wave_pattern_length;

    // Calculate level
    size_t offset_in_pattern = m_channel3.sample_point / 2;
    size_t offset_in_byte = 4 * (1 - (m_channel3.sample_point % 2));

    uint8_t byte = io_memory[WAVE_PATTERN_START + offset_in_pattern];
    uint8_t sample = (uint8_t)(byte >> offset_in_byte) & 0x0FU;

    switch (m_channel3.peek_level) {
      default:
        assert(!"Unreachable");
      case 0b00:
        m_channel3.current_output_level = 0;
        break;
      case 0b01:
        m_channel3.current_output_level = sample;
        break;
      case 0b10:
        m_channel3.current_output_level = sample >> 1U;
        break;
      case 0b11:
        m_channel3.current_output_level = sample >> 2U;
        break;
    }
  }
}

auto APU::tick_channel4() -> void {
  if (not m_channel4.channel_on) {
    return;
  }

  m_channel4.counter -= 1;
  if (m_channel4.counter == 0) {
    // Counter overflow, change sample point
    uint8_t config_reg = io_memory[CHANNEL4_RANDOMNESS];
    size_t divider = config_reg & 0b111U;
    size_t clock_shift = config_reg >> 4U;
    bool short_mode = (config_reg & (1U << 3U)) != 0;

    // Period = 4 * divider * (2 ** shift) increments
    size_t ticks_until_next_trigger = divider << (clock_shift + 2);
    m_channel4.counter = ticks_until_next_trigger;

    // Calculate the current noise
    bool current_bit0 = (m_channel4_lsr & 0b01U) != 0;
    bool current_bit1 = (m_channel4_lsr & 0b10U) != 0;
    bool new_bit15 = not(current_bit0 and current_bit1);

    // Update the register
    m_channel4_lsr >>= 1U;
    m_channel4_lsr |= ((size_t)new_bit15) << 15U;

    if (short_mode) {
      m_channel4_lsr |= ((size_t)new_bit15) << 7U;
    }

    assert(m_channel4.peek_level <= 0x0FU);
    m_channel4.current_output_level =
        (m_channel4_lsr & 1U) == 0U ? 0U : m_channel4.peek_level;
  }
}

auto APU::get_samples() -> std::span<std::pair<float, float>> {
  return m_samples_since_last_flush;
}

auto APU::flush_samples(size_t count) -> void {
  assert(count <= m_samples_since_last_flush.size());

  if (count == m_samples_since_last_flush.size()) {
    m_samples_since_last_flush.clear();
    return;
  }

  size_t size_after_flush = m_samples_since_last_flush.size() - count;
  std::copy(
      m_samples_since_last_flush.begin() + (long)count,
      m_samples_since_last_flush.begin() + (long)count + (long)size_after_flush,
      m_samples_since_last_flush.begin());
  m_samples_since_last_flush.resize(size_after_flush);
}

auto APU::envelope_sweep_event(APU::BasicChannel& channel) -> void {
  if (channel.envelope_sweep_pace != 0) {
    // The volume envelope is enabled
    if (--channel.envelope_timer == 0) {
      // The volume envelope has been fired
      // Note: the volume cannot over- or under-flow, the current volume is
      // not reflected in any memory-mapped register
      channel.envelope_timer = channel.envelope_sweep_pace;
      if (channel.envelope_increases) {
        channel.peek_level = std::min(channel.peek_level + 1, 0x0FUL);
      } else {
        channel.peek_level =
            channel.peek_level == 0 ? 0 : channel.peek_level - 1;
      }
    }
  }
}

auto APU::sound_length_event_channel3() -> void {
  auto length_enabled = (io_memory[CHANNEL3_CTL] & 0x40U) != 0;
  if (length_enabled) {
    m_channel3.length_timer += 1;
    if (m_channel3.length_timer == 256) {
      m_channel3.channel_on = false;
      m_channel3.length_timer = 0;
    }
    io_memory[CHANNEL3_LENGTH_TIMER] = m_channel3.length_timer;
  }
}

auto APU::sound_length_event(APU::BasicChannel& channel) -> void {
  auto length_enabled = (io_memory[channel.regs.ctl] & 0x40U) != 0;
  if (length_enabled) {
    channel.length_timer += 1;
    if (channel.length_timer == 64) {
      channel.channel_on = false;
      channel.length_timer = 0;
    }
    io_memory[channel.regs.length] =
        (io_memory[channel.regs.length] & 0xC0U) | channel.length_timer;
  }
}

auto APU::ch1_freq_sweep_event() -> void {
  uint8_t sweep_cfg_reg = io_memory[CHANNEL1_SWEEP];

  if (not m_channel1.channel_on) {
    return;
  }

  if (m_channel1_sweep_countdown != 0) {
    // The timer fires repeatedly even when disabled
    m_channel1_sweep_countdown -= 1;
  }

  if (m_channel1_sweep_countdown == 0 && m_channel1_sweep_active) {
    // Fire the sweep iteration logic
    size_t sweep_step = (sweep_cfg_reg & 0b111U);
    uint16_t new_period = channel1_sweep_calculate_or_overflow();

    // For some reason, step == 0 won't issue updates
    if (m_channel1_sweep_active && sweep_step != 0) {
      write_channel_period(io_memory, CHANNEL1_PERIOD_LOW, CHANNEL1_CTL,
                           new_period);
    }

    rearm_channel1_sweep();
  }
}

auto APU::rearm_channel1_sweep() -> void {
  uint8_t sweep_cfg_reg = io_memory[CHANNEL1_SWEEP];

  m_channel1_sweep_countdown = (uint8_t)(sweep_cfg_reg >> 4U) & 0b111U;
  m_channel1_sweep_active = m_channel1_sweep_countdown != 0;

  size_t sweep_step = (sweep_cfg_reg & 0b111U);

  m_channel1_sweep_locked_until_trigger =
      m_channel1_sweep_active || (!m_channel1_sweep_active && sweep_step == 0);

  if (sweep_step != 0) {
    // Do the overflow test immediately
    channel1_sweep_calculate_or_overflow();
  }
}

auto APU::channel1_sweep_calculate_or_overflow() -> size_t {
  uint8_t sweep_cfg_reg = io_memory[CHANNEL1_SWEEP];

  bool sweep_is_positive = (sweep_cfg_reg & (1U << 3U)) == 0;
  size_t sweep_step = (sweep_cfg_reg & 0b111U);
  uint16_t channel1_period =
      read_channel_period(io_memory, CHANNEL1_PERIOD_LOW, CHANNEL1_CTL);

  uint16_t period_delta = channel1_period >> sweep_step;
  if (sweep_is_positive) {
    if (channel1_period + period_delta >= (1U << 11U)) {
      // Positive overflow, disable channel 1
      m_channel1.channel_on = false;
      m_channel1_sweep_active = false;
    }
    return channel1_period + period_delta;
  }
  return channel1_period - period_delta;
}

auto APU::trigger_channel3() -> void {
  // TODO: what happens if the channel is triggered THEN the DAC is enabled?
  bool is_dac_enabled = (io_memory[CHANNEL3_DAC] & 0x80U) != 0;
  m_channel3.channel_on = is_dac_enabled;
  m_channel3.sample_point = 0;
  m_channel3.counter = 0;
  rearm_channel3_volume();
}

auto APU::trigger_pwm_channel(BasicChannel& channel) -> void {
  // TODO: what happens if the channel is triggered THEN the DAC is enabled?
  bool is_dac_enabled = (io_memory[channel.regs.volume_envelope] >> 3U) != 0;
  channel.channel_on = is_dac_enabled;
  channel.sample_point = 0;
  channel.counter = 0;
  if (&channel == &m_channel1) {
    rearm_channel1_sweep();
  }
  if (&channel == &m_channel4) {
    m_channel4_lsr = 0;
  }
  rearm_pwm_channel_volume_envelope(channel);
  rearm_pwm_channel_length(channel);
}

auto APU::rearm_pwm_channel_length(BasicChannel& channel) -> void {
  channel.length_timer = io_memory[channel.regs.length] & 0b111111U;
}

auto APU::rearm_channel3_length() -> void {
  m_channel3.length_timer = io_memory[CHANNEL3_LENGTH_TIMER];
}

auto APU::rearm_pwm_channel_volume_envelope(BasicChannel& channel) -> void {
  uint8_t volume_cfg_reg = io_memory[channel.regs.volume_envelope];
  channel.envelope_sweep_pace = (volume_cfg_reg & 0b111U);
  channel.envelope_timer = channel.envelope_sweep_pace;
  channel.envelope_increases = (volume_cfg_reg & 0b1000U) != 0;
  channel.peek_level = (volume_cfg_reg >> 4U);
}

auto APU::rearm_channel3_volume() -> void {
  m_channel3.peek_level = (uint8_t)(io_memory[CHANNEL3_VOLUME] >> 5U) & 0b11U;
}

auto APU::power_up() -> void {
  /*
  Powers up the APU while maintaining register values.
  This enables all writing.
  */
  m_apu_has_power = true;
  io_memory[AUDIO_MASTER_CTL] |= 0x80U;
}

auto APU::power_down() -> void {
  /*
  Powers down the APU and clears all related sound registers.
  This disables all writing until APU is powered up again
  */
  m_apu_has_power = false;
  io_memory[AUDIO_MASTER_CTL] &= 0x7FU;
  std::fill(io_memory.begin() + FIRST_APU_REGISTER,
            io_memory.begin() + LAST_APU_REGISTER, 0);
}

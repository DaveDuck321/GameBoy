#include "sdl_io.hpp"

#include "../libgb/io/io.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <numeric>
#include <optional>
#include <thread>
#include <utility>

using namespace std::chrono;

static constexpr std::array<std::array<uint8_t, 3>, 5> colorsRGB{
    {{{236, 247, 207}},
     {{145, 204, 120}},
     {{47, 116, 86}},
     {{8, 24, 28}},
     {{255, 0, 0}}}};

namespace {
auto rgb_to_uint32_t(const std::array<uint8_t, 3>& rgb) -> uint32_t {
  return (rgb[0] << 0U) | (rgb[1] << 8U) | (rgb[2] << 16U) | (255U << 24U);
}
}  // namespace

SDLFrontend::SDLFrontend() : m_key_events{{gb::Key::NONE}} {
  m_render_thread = std::thread{[&] {
    SDL_Init(SDL_INIT_EVERYTHING);
    m_window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, 3 * gb::SCREEN_WIDTH,
                                3 * gb::SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);

    SDL_RenderSetScale(m_renderer, 3.0F, 3.0F);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);

    {
      std::lock_guard _{m_render_mutex};
      m_textures = {SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      gb::SCREEN_WIDTH, gb::SCREEN_HEIGHT),
                    SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      gb::SCREEN_WIDTH, gb::SCREEN_HEIGHT)};

      // Initial writes will initialize the inactive texture
      SDL_LockTexture(m_textures[(m_live_texture + 1) % 2], nullptr,
                      (void**)&m_data_to_render, &m_render_stride);
    }
    m_render_buffer_ready.notify_all();

    async_render_loop();

    {
      std::lock_guard _{m_render_mutex};
      SDL_DestroyTexture(m_textures[0]);
      SDL_DestroyTexture(m_textures[1]);
    }
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
  }};

  // Wait for SDL to initialize
  std::unique_lock lock{m_render_mutex};
  m_render_buffer_ready.wait(lock, [&] { return m_data_to_render != nullptr; });

  SDL_AudioSpec desired_audio_spec = {
      .freq = 24000,
      .format = AUDIO_F32,
      .channels = 2,
      .silence = 0,
      .samples = 1024,
      .padding = 0,
      .size = 0,
      .callback = nullptr,
      .userdata = nullptr,
  };
  SDL_AudioSpec actual_audio_spec = {};

  m_audio_device = SDL_OpenAudioDevice(
      nullptr, 0, &desired_audio_spec, &actual_audio_spec,
      SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
  SDL_PauseAudioDevice(m_audio_device, 0);

  m_audio_sample_frequency = actual_audio_spec.freq;

  double sampling_period = 1.0 / (double)actual_audio_spec.freq;
  double lowpass_cutoff = 12000.0;
  double highpass_cutoff = 20.0;
  m_lowpass_alpha =
      (2.0 * std::numbers::pi * sampling_period * lowpass_cutoff) /
      (2.0 * std::numbers::pi * sampling_period * lowpass_cutoff + 1.0);
  m_highpass_alpha =
      1.0 / (1 + 2.0 * std::numbers::pi * highpass_cutoff * sampling_period);
  m_sample_buffer.resize(8UL * actual_audio_spec.samples);
}

auto SDLFrontend::process_events() -> void {
  auto map_to_GB_key = [](unsigned sdl_key) -> gb::Key {
    switch (sdl_key) {
      default:
        return gb::Key::NONE;
      case SDLK_LEFT:
        return gb::Key::LEFT;
      case SDLK_RIGHT:
        return gb::Key::RIGHT;
      case SDLK_UP:
        return gb::Key::UP;
      case SDLK_DOWN:
        return gb::Key::DOWN;
      case SDLK_w:
        return gb::Key::A;
      case SDLK_q:
        return gb::Key::B;
      case SDLK_RETURN:
        return gb::Key::SELECT;
      case SDLK_SPACE:
        return gb::Key::START;
    }
  };

  std::lock_guard _{m_keypress_mutex};
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    gb::Key& top_key = m_key_events.back();
    switch (event.type) {
      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_s) {
          m_speed_up_mode.store(true, std::memory_order_relaxed);
        }

        const auto pressed = map_to_GB_key(event.key.keysym.sym);
        if (m_last_keypress_was_down) {
          top_key = gb::Key(std::to_underlying(top_key) |
                            std::to_underlying(pressed));
        } else {
          m_key_events.push(gb::Key(std::to_underlying(top_key) |
                                    std::to_underlying(pressed)));
        }
        break;
      }
      case SDL_KEYUP: {
        if (event.key.keysym.sym == SDLK_s) {
          m_speed_up_mode.store(false, std::memory_order_relaxed);
        }

        const auto pressed = map_to_GB_key(event.key.keysym.sym);
        if (not m_last_keypress_was_down) {
          top_key = gb::Key(std::to_underlying(top_key) &
                            (uint8_t)~std::to_underlying(pressed));
        } else {
          m_key_events.emplace(gb::Key(std::to_underlying(top_key) &
                                       (uint16_t)~std::to_underlying(pressed)));
        }
        break;
      }
      case SDL_QUIT:
        m_exit_requested = true;
        break;
      default:
        break;
    }
  }
}

auto SDLFrontend::draw_frame() -> void {
  {
    std::mutex m_render_mutex;
    if (m_data_to_render == nullptr) {
      // The emulator has filled the inactive texture, lock and swap
      SDL_LockTexture(m_textures[m_live_texture], nullptr,
                      (void**)&m_data_to_render, &m_render_stride);

      // We're done, unlock asap and start copying the new active texture
      m_render_mutex.unlock();
      m_render_buffer_ready.notify_all();

      m_live_texture = (m_live_texture + 1) % 2;
      SDL_UnlockTexture(m_textures[m_live_texture]);
    } else {
      m_render_mutex.unlock();

      // The emulator hasn't finished preparing our next frame
      SDL_SetWindowTitle(m_window, "Emulator lagging render");
      m_last_lag_frame = m_real_frame_count;
    }
  }

  if (m_real_frame_count - m_last_lag_frame == 60) {
    // Hide lag frame message after 1s
    SDL_SetWindowTitle(m_window, "Running");
  }

  m_real_frame_count += 1;

  SDL_RenderClear(m_renderer);
  SDL_RenderCopy(m_renderer, m_textures[m_live_texture], nullptr, nullptr);
  SDL_RenderPresent(m_renderer);
}

auto SDLFrontend::async_render_loop() -> void {
  while (not isExitRequested()) {
    process_events();
    draw_frame();
  }
}

auto SDLFrontend::getKeyPressState() -> gb::Key {
  std::lock_guard _{m_keypress_mutex};
  if (m_key_events.size() > 1) {
    const auto key_press = m_key_events.front();
    m_key_events.pop();
    return key_press;
  }
  return m_key_events.front();
}

auto SDLFrontend::sendSerial(uint8_t value) -> void {
  std::cout << std::hex << value;
}

auto SDLFrontend::addPixel(int color, int screenX, int screenY) -> void {
  m_data_to_render[screenY * (m_render_stride / 4) + screenX] =
      rgb_to_uint32_t(colorsRGB[color]);
}

auto SDLFrontend::commitRender() -> void {
  m_gb_frame_count += 1;

  if (not m_speed_up_mode.load(std::memory_order_relaxed)) {
    m_current_frame_is_visible = true;
    m_data_to_render = nullptr;
    std::unique_lock lock{m_render_mutex};
    m_render_buffer_ready.wait(lock,
                               [&] { return m_data_to_render != nullptr; });
    return;
  }

  // speedup mode does not wait for a fresh frame buffer
  std::unique_lock lock{m_render_mutex};
  if (m_current_frame_is_visible) {
    // We've just rendered a frame, don't block -- skip next frame
    m_data_to_render = nullptr;
    m_current_frame_is_visible = false;
  } else {
    // We've just skipped a frame, check if we've got a frame buffer
    m_current_frame_is_visible = m_data_to_render != nullptr;
  }
}

auto SDLFrontend::isFrameScheduled() -> bool {
  return m_current_frame_is_visible;
}

auto SDLFrontend::isExitRequested() -> bool {
  return m_exit_requested.load(std::memory_order_relaxed);
}

auto SDLFrontend::get_approx_audio_sample_freq() -> size_t {
  return m_audio_sample_frequency;
}

auto SDLFrontend::try_flush_audio(std::span<std::pair<float, float>> samples)
    -> std::optional<size_t> {
  size_t sample_batch = m_sample_buffer.size() / 8;

  // Buffer ahead two batches of samples (for better discard behaviour)
  if (samples.size() < 2 * sample_batch) {
    return std::nullopt;
  }

  const auto samples_per_frame = m_audio_sample_frequency / 60;

  // Are we more than 2 frames ahead? If so start discarding some extra data to
  // catch up.
  size_t extra_discard = 0;
  size_t discard_every = sample_batch;
  if (auto queued_samples =
          (SDL_GetQueuedAudioSize(m_audio_device) / (2 * sizeof(float)));
      queued_samples > 2 * samples_per_frame) {
    extra_discard += 5;
    discard_every = sample_batch / extra_discard;

    // Are we more than 3 frames ahead?? Give up and flush everything until we
    // recover.
    if (queued_samples > 5 * samples_per_frame) {
      return samples.size();
    }
  }

  size_t gameboy_sample = 0;
  for (size_t host_index = 0; host_index < sample_batch; host_index += 1) {
    gameboy_sample += 1;
    if (gameboy_sample % discard_every == 0) {
      gameboy_sample += 1;
    }

    size_t gameboy_sample = host_index;
    double raw_l = samples[gameboy_sample].first;
    double raw_r = samples[gameboy_sample].first;

    double high_l = m_highpass_alpha * (m_highpass_l.last_output + raw_l -
                                        m_highpass_l.last_input);
    double high_r = m_highpass_alpha * (m_highpass_r.last_output + raw_r -
                                        m_highpass_r.last_input);
    m_highpass_l.last_input = raw_l;
    m_highpass_r.last_input = raw_r;
    m_highpass_l.last_output = high_l;
    m_highpass_r.last_output = high_r;

    double sample_l = m_highpass_l.last_output +
                      m_lowpass_alpha * (high_l - m_highpass_l.last_output);
    double sample_r = m_highpass_r.last_output +
                      m_lowpass_alpha * (high_r - m_highpass_r.last_output);
    m_lowpass_l.last_input = high_l;
    m_lowpass_r.last_input = high_r;
    m_lowpass_l.last_output = sample_l;
    m_lowpass_r.last_output = sample_r;

    m_sample_buffer[2 * host_index] = (float)sample_l;
    m_sample_buffer[2 * host_index + 1] = (float)sample_r;
  }
  SDL_QueueAudio(m_audio_device, m_sample_buffer.data(),
                 m_sample_buffer.size());
  return sample_batch + extra_discard;
}

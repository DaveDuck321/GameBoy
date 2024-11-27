#pragma once

#include "../libgb/constants.hpp"
#include "../libgb/io/frontend.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_render.h>
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <queue>
#include <vector>

class SDLFrontend : public gb::IOFrontend {
  std::thread m_render_thread;

  // Rendering
  SDL_Window* m_window = nullptr;
  SDL_Renderer* m_renderer = nullptr;
  SDL_AudioDeviceID m_audio_device = 0;
  size_t m_live_texture = 0;
  std::array<SDL_Texture*, 2> m_textures = {nullptr, nullptr};

  std::mutex m_render_mutex;
  std::condition_variable m_render_buffer_ready;

  uint32_t* m_data_to_render = nullptr;
  int m_render_stride = 0;
  bool m_current_frame_is_visible = true;

  size_t m_audio_sample_frequency = 0;

  // State for the highpass/ lowpass iir
  struct IIRFilter {
    double last_output;
    double last_input;
  };

  IIRFilter m_highpass_r = {};
  IIRFilter m_highpass_l = {};
  IIRFilter m_lowpass_r = {};
  IIRFilter m_lowpass_l = {};
  double m_lowpass_alpha = 0;
  double m_highpass_alpha = 0;

  std::vector<float> m_sample_buffer;

  // Inputs
  std::mutex m_keypress_mutex;
  std::queue<gb::Key> m_key_events;
  bool m_last_keypress_was_down = false;

  std::atomic<bool> m_speed_up_mode = false;
  std::atomic<bool> m_exit_requested = false;

  // Diagnostics
  size_t m_gb_frame_count = 0;
  size_t m_real_frame_count = 0;
  size_t m_last_lag_frame = 0;

 public:
  SDLFrontend();
  SDLFrontend(const SDLFrontend&) = delete;
  auto operator=(const SDLFrontend&) -> SDLFrontend& = delete;
  ~SDLFrontend() override = default;

  auto process_events() -> void;
  auto draw_frame() -> void;
  auto async_render_loop() -> void;

  auto getKeyPressState() -> gb::Key override;
  auto sendSerial(uint8_t value) -> void override;

  auto addPixel(int color, int screenX, int screenY) -> void override;
  auto commitRender() -> void override;
  auto isFrameScheduled() -> bool override;
  auto isExitRequested() -> bool override;

  auto get_approx_audio_sample_freq() -> size_t override;
  auto try_flush_audio(std::span<std::pair<float, float>> samples)
      -> std::optional<size_t> override;
};

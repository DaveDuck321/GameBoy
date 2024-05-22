#pragma once

#include "../libgb/constants.hpp"
#include "../libgb/io/frontend.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <queue>

class SDLFrontend : public gb::IOFrontend {
  // -- SDL rendering state
  SDL_Window* m_window = nullptr;
  SDL_Renderer* m_renderer = nullptr;
  size_t m_live_texture = 0;
  std::array<SDL_Texture*, 2> m_textures = {nullptr, nullptr};

  std::mutex m_render_mutex;
  std::condition_variable m_render_buffer_ready;
  uint32_t* m_data_to_render = nullptr;
  int m_render_stride = 0;

  std::thread m_render_thread;
  // -- End SDL rendering state

  // Never drop a keypress, record everything
  std::mutex m_keypress_mutex;
  std::queue<gb::Key> m_key_events;
  bool m_last_keypress_was_down = false;
  std::atomic<bool> m_exit_requested = false;

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
};

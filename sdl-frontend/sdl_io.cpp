#include "sdl_io.hpp"

#include "../libgb/io/io.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <mutex>
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
      // The GB has filled the inactive texture, lock and swap
      SDL_LockTexture(m_textures[m_live_texture], nullptr,
                      (void**)&m_data_to_render, &m_render_stride);

      // We're done, unlock asap and start copying the new active texture
      m_render_mutex.unlock();
      m_render_buffer_ready.notify_all();

      m_live_texture = (m_live_texture + 1) % 2;
      SDL_UnlockTexture(m_textures[m_live_texture]);
    }
  }

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
  m_data_to_render = nullptr;
  std::unique_lock lock{m_render_mutex};
  m_render_buffer_ready.wait(lock, [&] { return m_data_to_render != nullptr; });
}

auto SDLFrontend::isFrameScheduled() -> bool {
  return true;
}

auto SDLFrontend::isExitRequested() -> bool {
  return m_exit_requested;
}

#pragma once

#include "../libgb/io/frontend.hpp"

#include <SDL2/SDL.h>
#include <chrono>

class SDLFrontend : public gb::IOFrontend {
  // SDL stuff is more effort with smart pointers
  SDL_Window* window;
  SDL_Renderer* renderer;

  // Used for speedup mode
  bool realtime = true;
  bool lagframe = false;

  bool frameScheduled = true;
  bool exitRequested = false;
  uint32_t frameCountSinceLastFPSMeasurement = 0;

  double sleepDrift = 0;
  // Clock for FPS limiter
  std::chrono::high_resolution_clock::time_point lastFrame;
  // Clock for FPS update
  std::chrono::high_resolution_clock::time_point lastFPSUpdate;
  // Clock for speed up
  std::chrono::high_resolution_clock::time_point lastRenderedFrame;

  gb::Key keyState;

 public:
  SDLFrontend();
  SDLFrontend(const SDLFrontend&) = delete;
  auto operator=(const SDLFrontend&) -> SDLFrontend& = delete;
  ~SDLFrontend() override;

  auto getKeyPressState() -> gb::Key override;
  auto sendSerial(uint8_t value) -> void override;

  auto addPixel(int color, int screenX, int screenY) const -> void override;
  auto commitRender() -> void override;
  auto isFrameScheduled() -> bool override;
  auto isExitRequested() -> bool override;
};

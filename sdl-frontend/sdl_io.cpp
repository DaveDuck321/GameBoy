#include "sdl_io.hpp"

#include <SDL2/SDL.h>
#include "../libgb/constants.hpp"
#include "../libgb/io/io.hpp"

#include <array>
#include <iostream>
#include <sstream>

#include <thread>
#include <utility>

using namespace std::chrono;

static constexpr std::array<std::array<int, 3>, 5> colorsRGB{{{{236, 247, 207}},
                                                              {{145, 204, 120}},
                                                              {{47, 116, 86}},
                                                              {{8, 24, 28}},
                                                              {{255, 0, 0}}}};

auto delayTime(double time) -> void {
  /*
  Pauses the current thread's execution for 'time' seconds.
  */
  // Good efficient code if its supported by the platform
  std::this_thread::sleep_for(duration<double>(time));
}

auto calculateSleepDrift(double testTime) -> double {
  /*
  Attempts to calculate the time required to call 'sleep_for'.
  This result can be used to adjust timer
  */
  // sleep_for function takes a non-negligible amount of time. Measure it
  auto start = high_resolution_clock::now();

  std::this_thread::sleep_for(duration<double>(testTime));

  duration<double> time = high_resolution_clock::now() - start;

  return time.count() - testTime;
}

SDLFrontend::SDLFrontend()
    : sleepDrift(calculateSleepDrift(1.0 / 120.0)),
      lastFrame(high_resolution_clock::now()),
      lastFPSUpdate(lastFrame),
      lastRenderedFrame(lastFPSUpdate),
      keyState(gb::Key::NONE) {
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 3 * gb::SCREEN_WIDTH,
                            3 * gb::SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_RenderSetScale(renderer, 3.0F, 3.0F);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
}

SDLFrontend::~SDLFrontend() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

auto SDLFrontend::getKeyPressState() -> gb::Key {
  high_resolution_clock::time_point time = high_resolution_clock::now();
  if (!realtime) {
    // In speedup mode, only render to screen @ vsync
    duration<double> timeSinceRender = time - lastRenderedFrame;
    // About 60 frames per second
    frameScheduled = timeSinceRender.count() > 1.0 / 60.0;
  }

  // Update FPS every half second
  duration<double> timeSinceUpdate = time - lastFPSUpdate;
  if (timeSinceUpdate.count() > 0.5) {
    std::stringstream title;

    double fps = frameCountSinceLastFPSMeasurement / timeSinceUpdate.count();
    title << "SDL frontend :: FPS: " << fps;
    if (lagframe) {
      title << " -- lagging";
    }

    SDL_SetWindowTitle(window, title.str().c_str());
    // Reset counters
    frameCountSinceLastFPSMeasurement = 0;
    lagframe = false;
    lastFPSUpdate = time;
  }

  // Keyboard and quit events
  auto mapToGBKey = [](unsigned sdlKey) {
    switch (sdlKey) {
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

  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_s) {
          // Toggle speedup mode and render next frame
          realtime = !realtime;
          frameScheduled = true;
        }

        auto pressed = mapToGBKey(event.key.keysym.sym);
        keyState =
            gb::Key(std::to_underlying(keyState) | std::to_underlying(pressed));
        break;
      }
      case SDL_KEYUP: {
        auto pressed = mapToGBKey(event.key.keysym.sym);
        keyState = gb::Key(std::to_underlying(keyState) &
                           ~std::to_underlying(pressed));
        break;
      }
      case SDL_QUIT:
        exitRequested = true;
      default:
        break;
    }
  }
  return keyState;
}

auto SDLFrontend::sendSerial(uint8_t value) -> void {
  std::cout << std::hex << value;
}

auto SDLFrontend::addPixel(int color, int screenX, int screenY) const -> void {
  SDL_SetRenderDrawColor(renderer, colorsRGB[color][0], colorsRGB[color][1],
                         colorsRGB[color][2], 255);
  SDL_RenderDrawPoint(renderer, screenX, screenY);
}

auto SDLFrontend::commitRender() -> void {
  frameCountSinceLastFPSMeasurement++;
  if (realtime) {
    // Shouldn't render at a higher framerate than the gameboy
    duration<double> frameDuration = high_resolution_clock::now() - lastFrame;
    double dt = frameDuration.count() + sleepDrift;

    if (dt <= gb::FRAMETIME) {
      delayTime(gb::FRAMETIME - dt);
    } else {
      lagframe = true;
    }

    lastFrame = high_resolution_clock::now();
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
  } else if (frameScheduled) {
    // Record frame time and render to display
    frameScheduled = false;
    lastRenderedFrame = high_resolution_clock::now();
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
  }
}

auto SDLFrontend::isFrameScheduled() -> bool {
  return frameScheduled;
}

auto SDLFrontend::isExitRequested() -> bool {
  return exitRequested;
}

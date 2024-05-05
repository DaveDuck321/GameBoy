#pragma once

// Nspire doesn't support SDL2
#include <SDL/SDL.h>
#include <SDL/SDL_config.h>
#include <memory>
#include <os.h>

#include "io_manager.hpp"

class NSPIRE_SDL : public gb::IOFrontend {
private:
  SDL_Surface *screen;

public:
  NSPIRE_SDL();
  ~NSPIRE_SDL();

  void pollEvents() override;
  void sendSerial(uint8_t value) override{};

  void finishRender() override;
  void drawPixel(int color, int screenX, int screenY) const override;
};

#endif

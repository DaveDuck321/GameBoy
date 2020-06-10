#ifndef sdl_min_io_h
#define sdl_min_io_h

// Nspire doesn't support SDL2
#include <os.h>
#include <SDL/SDL_config.h>
#include <SDL/SDL.h>
#include <memory>

#include "io_manager.hpp"

class NSPIRE_SDL: public IO_Manager
{
    private:
    SDL_Surface *screen;

    public:
    NSPIRE_SDL();
    ~NSPIRE_SDL();

    void pollEvents() override;
    void sendSerial(uint8_t value) override {};
    
    void finishRender() override;
    void drawPixel(int color, int screenX, int screenY) const override;
};

#endif
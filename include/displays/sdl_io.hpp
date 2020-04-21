#ifndef sdl_io_h
#define sdl_io_h

#include <SDL2/SDL.h>
#include <memory>

#include "io_manager.hpp"

class SDL_IO: public IO_Manager
{
    private:
    // SDL stuff is more effort with smart pointers
    SDL_Window *window;
    SDL_Renderer *renderer;

    public:
    SDL_IO();
    ~SDL_IO();

    void pollEvents() override;
    void sendSerial(uint8_t value) override {};
    
    void finishRender() const override;
    void drawPixel(int color, int screenX, int screenY) const override;
};

#endif
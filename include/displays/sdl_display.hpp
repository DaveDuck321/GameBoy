#ifndef sdl_display_h
#define sdl_display_h

#include <SDL2/SDL.h>
#include <memory>

#include "io_manager.hpp"

class SDL_Display: public IO_Manager
{
    private:
    // SDL stuff is more effort with smart pointers
    SDL_Window *window;
    SDL_Renderer *renderer;

    public:
    SDL_Display();
    ~SDL_Display();

    void pollEvents() override;
    void finishRender() const override;

    void drawPixel(int color, int screenX, int screenY) const override;
};

#endif
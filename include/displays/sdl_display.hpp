#ifndef sdl_display_h
#define sdl_display_h

#include <SDL2/SDL.h>
#include <memory>

#include "display.hpp"

class SDL_Display: public Display
{
    private:
    // SDL stuff is more effort with smart pointers
    SDL_Window *window;
    SDL_Renderer *renderer;

    public:
    SDL_Display();
    ~SDL_Display();

    void clearScreen() const override;
    void finishRender() const override;

    void drawTile(Tile tile, int screenX, int screenY) const;
};

#endif
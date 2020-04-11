#include "displays/sdl_display.hpp"

#include <iostream>


SDL_Display::SDL_Display()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        "Gameboy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        600, 400,
        SDL_WINDOW_SHOWN
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
}

SDL_Display::~SDL_Display()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void SDL_Display::finishRender() const
{
    SDL_RenderPresent(renderer);

    SDL_Event event;
    SDL_PollEvent(&event);
}

void SDL_Display::drawPixel(int color, int screenX, int screenY) const
{
    int intensity = (((float)color)/3.0f) * 255;

    SDL_SetRenderDrawColor(renderer, intensity, intensity, intensity, 255);
    SDL_RenderDrawPoint(renderer, screenX, screenY);
}

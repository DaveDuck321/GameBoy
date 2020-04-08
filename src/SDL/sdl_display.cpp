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

void SDL_Display::clearScreen() const
{
    SDL_Event event;
    SDL_PollEvent(&event);

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void SDL_Display::finishRender() const
{
    SDL_RenderPresent(renderer);
}

void SDL_Display::drawTile(Tile tile, int screenX, int screenY) const
{
    for(int offsetY=0; offsetY<8; offsetY++)
    {
        uint8_t lowerbits = tile[offsetY][0];
        uint8_t upperbits = tile[offsetY][1];
        for(int offsetX=7; offsetX>=0; offsetX--)
        {
            int pixel = (lowerbits&1) | ((upperbits&1)<<1);
            int intensity = (((float)pixel)/3.0f) * 255;

            SDL_SetRenderDrawColor(renderer, intensity, intensity, intensity, 255);
            SDL_RenderDrawPoint(renderer, screenX+offsetX, screenY+offsetY);

            lowerbits >>= 1;
            upperbits >>= 1;
        }
    }
}

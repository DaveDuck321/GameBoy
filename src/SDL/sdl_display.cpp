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

void SDL_Display::pollEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        auto keyAction = &SDL_Display::releaseKey;
        switch(event.type)
        {
        case SDL_KEYDOWN:
            keyAction = &SDL_Display::pressKey;
        case SDL_KEYUP:
            switch (event.key.keysym.scancode)
            {
            case SDL_SCANCODE_LEFT:
                (this->*keyAction)(Key::LEFT);
                break;
            case SDL_SCANCODE_RIGHT:
                (this->*keyAction)(Key::RIGHT);
                break;
            case SDL_SCANCODE_UP:
                (this->*keyAction)(Key::UP);
                break;
            case SDL_SCANCODE_DOWN:
                (this->*keyAction)(Key::DOWN);
                break;
            case SDL_SCANCODE_W:
                (this->*keyAction)(Key::A);
                break;
            case SDL_SCANCODE_Q:
                (this->*keyAction)(Key::B);
                break;
            case SDL_SCANCODE_RETURN:
                (this->*keyAction)(Key::SELECT);
                break;
            case SDL_SCANCODE_SPACE:
                (this->*keyAction)(Key::START);
                break;
            default:
                break;
            }
            break;
        case SDL_QUIT:
            // Hacky exit
            throw EXIT_SUCCESS;
        default:
            break;
        }
    }
}

void SDL_Display::finishRender() const
{
    SDL_RenderPresent(renderer);
}

void SDL_Display::drawPixel(int color, int screenX, int screenY) const
{
    int intensity = (((float)color)/3.0f) * 255;

    SDL_SetRenderDrawColor(renderer, intensity, intensity, intensity, 255);
    SDL_RenderDrawPoint(renderer, screenX, screenY);
}


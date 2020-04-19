#include "displays/sdl_display.hpp"

#include <iostream>
#include <array>

std::array<std::array<int, 3>, 4> colorsRGB {{
    {{181, 229, 6}},
    {{139, 172, 15}},
    {{48, 98, 48}},
    {{15, 56, 15}}
}};

SDL_Display::SDL_Display()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        "Gameboy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        2*SCREEN_WIDTH, 2*SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetScale(renderer, 2.0f, 2.0f);
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
    SDL_SetRenderDrawColor(renderer, colorsRGB[color][0], colorsRGB[color][1], colorsRGB[color][2], 255);
    SDL_RenderDrawPoint(renderer, screenX, screenY);
}


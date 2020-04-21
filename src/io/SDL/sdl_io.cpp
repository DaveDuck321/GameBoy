#include "displays/sdl_io.hpp"

#include <iostream>
#include <array>

std::array<std::array<int, 3>, 5> colorsRGB {{
    {{236, 247, 207}},
    {{145, 204, 120}},
    {{47, 116, 86}},
    {{8, 24, 28}},
    {{255, 0, 0}}
}};


SDL_IO::SDL_IO()
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

SDL_IO::~SDL_IO()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void SDL_IO::pollEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        auto keyAction = &SDL_IO::releaseKey;
        switch(event.type)
        {
        case SDL_KEYDOWN:
            keyAction = &SDL_IO::pressKey;
        case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
            case SDLK_LEFT:
                (this->*keyAction)(Key::LEFT);
                break;
            case SDLK_RIGHT:
                (this->*keyAction)(Key::RIGHT);
                break;
            case SDLK_UP:
                (this->*keyAction)(Key::UP);
                break;
            case SDLK_DOWN:
                (this->*keyAction)(Key::DOWN);
                break;
            case SDLK_w:
                (this->*keyAction)(Key::A);
                break;
            case SDLK_q:
                (this->*keyAction)(Key::B);
                break;
            case SDLK_RETURN:
                (this->*keyAction)(Key::SELECT);
                break;
            case SDLK_SPACE:
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

void SDL_IO::finishRender() const
{
    SDL_RenderPresent(renderer);
}

void SDL_IO::drawPixel(int color, int screenX, int screenY) const
{
    SDL_SetRenderDrawColor(renderer, colorsRGB[color][0], colorsRGB[color][1], colorsRGB[color][2], 255);
    SDL_RenderDrawPoint(renderer, screenX, screenY);
}

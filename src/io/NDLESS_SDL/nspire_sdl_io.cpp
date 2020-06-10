#include "displays/nspire_sdl.hpp"

#include <iostream>
#include <array>

std::array<std::array<int, 3>, 5> colorsRGB {{
    {{236, 247, 207}},
    {{145, 204, 120}},
    {{47, 116, 86}},
    {{8, 24, 28}},
    {{255, 0, 0}}
}};

NSPIRE_SDL::NSPIRE_SDL()
{
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(320, 240, has_colors ? 16 : 8, SDL_SWSURFACE);
}

NSPIRE_SDL::~NSPIRE_SDL()
{
    SDL_Quit();
}

void NSPIRE_SDL::pollEvents()
{
    // Checking for updates is slow, check every other frame
    if(frameScheduled)  return;

    // Keyboard and quit events
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        auto keyAction = &NSPIRE_SDL::releaseKey;
        switch(event.type)
        {
        case SDL_KEYDOWN:
            keyAction = &NSPIRE_SDL::pressKey;
            // Fall through
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
            case SDLK_ESCAPE:
                // Hacky exit
                throw EXIT_SUCCESS;
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

void NSPIRE_SDL::finishRender()
{
    // Render every other frame for 30fps (target) output
    if(frameScheduled)  SDL_Flip(screen);
    frameScheduled = !frameScheduled;
}

void NSPIRE_SDL::drawPixel(int color, int screenX, int screenY) const
{
    SDL_Rect rect {screenX, screenY, 1, 1};
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, colorsRGB[color][0], colorsRGB[color][1], colorsRGB[color][2]));
}

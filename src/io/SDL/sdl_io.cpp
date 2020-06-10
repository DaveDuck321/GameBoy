#include "displays/sdl_io.hpp"

#include <iostream>
#include <array>
#include <string>
#include <sstream>

#include <thread>

using namespace std::chrono;

std::array<std::array<int, 3>, 5> colorsRGB {{
    {{236, 247, 207}},
    {{145, 204, 120}},
    {{47, 116, 86}},
    {{8, 24, 28}},
    {{255, 0, 0}}
}};


void delayTime(double time)
{
    /*
    Pauses the current thread's execution for 'time' seconds.
    This is platform dependent: 'SDL_Delay' and 'sleep_for' don't work on Windows.
    */
#if defined(WIN32) || defined(_WIN32)
    // Mingw doesn't support sleep_for ?
    // Also Windows.h Sleep() takes FAR too long to achieve 60fps
    // This hacky solution's probably better than dll hell
    time_point start = high_resolution_clock::now();

    duration<double> elapsed = seconds(0);
    while(elapsed.count() < time)
    {
        elapsed = high_resolution_clock::now() - start;
    }
#else
    // Good efficient code if its supported by the platform
    std::this_thread::sleep_for(duration<double>(time));
#endif
}

double getClockDrift(double testTime)
{
    /*
    Attempts to calculate the time required to call 'sleep_for'.
    This result can be used to adjust timer
    */
#if defined(WIN32) || defined(_WIN32)
    // Windows has no drift since a loop is used
    return 0;
#else
    // sleep_for function takes a non-negligible amount of time. Measure it

    auto start = high_resolution_clock::now();

    std::this_thread::sleep_for(duration<double>(testTime));

    duration<double> time = high_resolution_clock::now() - start;

    return time.count() - testTime;
#endif
}

SDL_IO::SDL_IO()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        "Gameboy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        3*GB_SCREEN_WIDTH, 3*GB_SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetScale(renderer, 3.0f, 3.0f);

    lastFrame = high_resolution_clock::now();
    lastFPSUpdate = lastFrame;
    lastRenderedFrame = lastFPSUpdate;
    frameScheduled = true;
    realtime = true;

    clockDrift = getClockDrift(1.0 / 120.0);
}

SDL_IO::~SDL_IO()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void SDL_IO::pollEvents()
{
    high_resolution_clock::time_point time = high_resolution_clock::now();
    if(!realtime)
    {
        // In speedup mode, only render to screen @ vsync
        duration<double> timeSinceRender = time-lastRenderedFrame;
        //About 60 frames per second
        frameScheduled = timeSinceRender.count() > 1.0/60.0;
    }

    //Update FPS every half second
    duration<double> timeSinceUpdate = time-lastFPSUpdate;
    if(timeSinceUpdate.count()>0.5)
    {
        std::stringstream title;

        double fps = frames/timeSinceUpdate.count();
        title << gameTitle << " :: FPS: " << fps;
        if(lagframe)    title << " -- lagging";
        SDL_SetWindowTitle(window, title.str().c_str());
        // Reset counters
        frames = 0;
        lagframe = false;
        lastFPSUpdate = time;
    }

    // Keyboard and quit events
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        auto keyAction = &SDL_IO::releaseKey;
        switch(event.type)
        {
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_s)
            {
                // Toggle speedup mode and render next frame
                realtime = !realtime;
                frameScheduled = true;
            }
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

void SDL_IO::finishRender()
{
    frames++;
    if(realtime)
    {
        // Shouldn't render at a higher framerate than the gameboy
        duration<double> frameDuration = high_resolution_clock::now()-lastFrame;
        double dt = frameDuration.count() + clockDrift; // Account for clock inaccuracy
        
        if(dt <= FRAMETIME) delayTime(FRAMETIME-dt);
        else                lagframe = true;
        
        lastFrame = high_resolution_clock::now();
        SDL_RenderPresent(renderer);
    }
    else if(frameScheduled)
    {
        // Record frame time and render to display
        frameScheduled = false;
        lastRenderedFrame = high_resolution_clock::now();
        SDL_RenderPresent(renderer);
    }
}

void SDL_IO::drawPixel(int color, int screenX, int screenY) const
{
    SDL_SetRenderDrawColor(renderer, colorsRGB[color][0], colorsRGB[color][1], colorsRGB[color][2], 255);
    SDL_RenderDrawPoint(renderer, screenX, screenY);
}

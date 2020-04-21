#ifndef sdl_io_h
#define sdl_io_h

#include <SDL2/SDL.h>
#include <memory>
#include <chrono>

#include "io_manager.hpp"

class SDL_IO: public IO_Manager
{
    private:
    // SDL stuff is more effort with smart pointers
    SDL_Window *window;
    SDL_Renderer *renderer;

    //Used for speedup mode
    bool realtime = true;
    bool lagframe = false;
    uint32_t frames = 0;
    uint32_t lastFrameTime = 0;
    // Clock for FPS update
    std::chrono::high_resolution_clock::time_point lastFPSUpdate;
    // Clock for speed up
    std::chrono::high_resolution_clock::time_point lastRenderedFrame;

    public:
    SDL_IO();
    ~SDL_IO();

    void pollEvents() override;
    void sendSerial(uint8_t value) override {};
    
    void finishRender() override;
    void drawPixel(int color, int screenX, int screenY) const override;
};

#endif
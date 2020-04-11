#ifndef headless_h
#define headless_h

#include <memory>

#include "io_manager.hpp"

class Headless: public IO_Manager
{
    private:
    public:

    virtual void drawPixel(int color, int screenX, int screenY) const = 0;
};

#endif
#ifndef headless_h
#define headless_h

#include <memory>

#include "display.hpp"

class Headless: public Display
{
    private:
    public:

    void clearScreen() const override {};

    void drawTile(Tile tile, int screenX, int screenY) const {};
};

#endif
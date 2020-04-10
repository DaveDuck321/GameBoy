#ifndef headless_h
#define headless_h

#include <memory>

#include "io_manager.hpp"

class Headless: public IO_Manager
{
    private:
    public:

    void clearScreen() const override {};

    void drawTile(Tile tile, int screenX, int screenY) const {};
};

#endif
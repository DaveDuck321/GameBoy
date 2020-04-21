#ifndef headless_h
#define headless_h

#include "io_manager.hpp"

#include <memory>
#include <iostream>

class Headless: public IO_Manager
{
    private:
    public:
    std::ostream &os;
    Headless(std::ostream &os):os(os) {};
    
    void pollEvents() override {};
    void sendSerial(uint8_t value) override { os << value; };

    void drawPixel(int color, int screenX, int screenY) const override {};
};

#endif
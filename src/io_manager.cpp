#include "io_manager.hpp"

#include <iostream>

IO_Manager::IO_Manager()
{
    memory.fill(0); //For debuging
    // Set line count ready to wrap
    memory[LCD_LY] = 255;
}

uint8_t IO_Manager::videoRead(uint16_t addr) const
{
    switch(addr)
    {
        case 0x8000 ... 0x97FF:
            // Tile data 1
            // Flatten 3D array and write to it like the gameboy would
            // (Might cause errors on some compilers -- TODO: check)
            static_assert(sizeof(patternTables) == 0x1800);
            return (reinterpret_cast<const uint8_t *>(&patternTables))[addr-0x8000];
        case 0x9800 ... 0x9BFF:
            //Background map 1
            static_assert(sizeof(backgroundMap1) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap1))[addr-0x9800];
        case 0x9C00 ... 0x9FFF:
            //Background map 2
            static_assert(sizeof(backgroundMap2) == 0x400);
            return (reinterpret_cast<const uint8_t *>(&backgroundMap2))[addr-0x9C00];
        case 0xFE00 ... 0xFE9F:
            //Sprite attributes
            static_assert(sizeof(sprites) == 0xA0);
            return (reinterpret_cast<const uint8_t *>(&sprites))[addr-0xFE00];
        default:
            throw std::range_error("Bad vram address read");
    }
}

void IO_Manager::videoWrite(uint16_t addr, uint8_t value)
{
    switch(addr)
    {
        case 0x8000 ... 0x97FF:
            // Tile data 1
            static_assert(sizeof(patternTables) == 0x1800);
            (reinterpret_cast<uint8_t *>(&patternTables))[addr-0x8000] = value;
            break;
        case 0x9800 ... 0x9BFF:
            //Background map 1
            static_assert(sizeof(backgroundMap1) == 0x400);
            (reinterpret_cast<uint8_t *>(&backgroundMap1))[addr-0x9800] = value;
            break;
        case 0x9C00 ... 0x9FFF:
            //Background map 2
            static_assert(sizeof(backgroundMap2) == 0x400);
            (reinterpret_cast<uint8_t *>(&backgroundMap2))[addr-0x9C00] = value;
            break;
        case 0xFE00 ... 0xFE9F:
            //Sprite attributes
            static_assert(sizeof(sprites) == 0xA0);
            (reinterpret_cast<uint8_t *>(&sprites))[addr-0xFE00] = value;
            break;
        default:
            throw std::range_error("Bad vram address write");
    }
}

void IO_Manager::ioWrite(uint16_t addr, uint8_t value)
{
    uint8_t offsetAddr = addr-0xFF00;
    switch (addr)
    {
    case 0xFF00:
        //P1 -- joypad info (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF01:
        //SB -- Serial transfer data (r/w)
        //std::cout << "SB: "<< value <<std::endl;
        std::cout<<value;
        memory[offsetAddr] = value;
        break;
    case 0xFF02:
        //SC -- SIO control (r/w)
        //std::cout << "SC!" <<std::endl;
        memory[offsetAddr] = value;
        break;
    case 0xFF04:
        //DIV -- Divider Register (r/w)
        memory[offsetAddr] = 0;
        break;
    case 0xFF05:
        //TIMA -- Timer Counter (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF06:
        //TMA -- Timer Modulo (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF07:
        //TAC -- Timer Control (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF0F:
        //IF -- Interrupt Flag (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF10 ... 0xFF3F:
        //Sound stuff
        memory[offsetAddr] = value;
        break;
    case 0xFF40:
        //LCDC -- LCD Control (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF41:
        //STAT -- LCDC Status (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF42:
        //SCY -- Scroll Y (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF43:
        //SCX -- Scroll X (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF44:
        //LY -- Scroll Y (r)
        throw std::runtime_error("Cannot write to LY @ 0xFF44");
    case 0xFF45:
        //LYC -- LY Compare (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF46:
        //DMA -- DMA Transfer and start address (W)
        throw std::runtime_error("DMA Transfer not implemented!");
    case 0xFF47 ... 0xFF49:
        //BGP -- BG & Window palette data (r/w)
        //OBP0 -- Object Palette 0 data (r/w)
        //OBP1 -- Object Palette 1 data (r/w)
        memory[offsetAddr] = value;
        break;
    case 0xFF4A: case 0xFF4B:
        //Window x and y positions
        memory[offsetAddr] = value;
        break;
    
    default:
        throw std::runtime_error("Bad IO write address");
    }
}

uint8_t IO_Manager::ioRead(uint16_t addr) const
{
    uint8_t offsetAddr = addr-0xFF00;
    switch (addr)
    {
    case 0xFF00:
        //P1 -- joypad info (r/w)
        return memory[offsetAddr];
    case 0xFF01:
        //SB -- Serial transfer data (r/w)
        return memory[offsetAddr];
    case 0xFF02:
        //SC -- SIO control (r/w)
        return memory[offsetAddr];
    case 0xFF04:
        //DIV -- Divider Register (r/w)
        return memory[offsetAddr];
    case 0xFF05:
        //TIMA -- Timer Counter (r/w)
        return memory[offsetAddr];
    case 0xFF06:
        //TMA -- Timer Modulo (r/w)
        return memory[offsetAddr];
    case 0xFF07:
        //TAC -- Timer Control (r/w)
        return memory[offsetAddr];
    case 0xFF0F:
        //IF -- Interrupt Flag (r/w)
        return memory[offsetAddr];
    case 0xFF10 ... 0xFF3F:
        //Sound stuff
        return memory[offsetAddr];
    case 0xFF40:
        //LCDC -- LCD Control (r/w)
        return memory[offsetAddr];
    case 0xFF41:
        //STAT -- LCDC Status (r/w)
        return memory[offsetAddr];
    case 0xFF42:
        //SCY -- Scroll Y (r/w)
        return memory[offsetAddr];
    case 0xFF43:
        //SCX -- Scroll X (r/w)
        return memory[offsetAddr];
    case 0xFF44:
        //LY -- Scroll Y (r)
        // TODO implement LY
        //std::cout << "LY read requested" << std::endl;
        return memory[offsetAddr];
    case 0xFF45:
        //LYC -- LY Compare (r/w)
        return memory[offsetAddr];
    case 0xFF46:
        //DMA -- DMA Transfer and start address (W)
        throw std::runtime_error("DMA read not permitted!");
    case 0xFF47 ... 0xFF49:
        //BGP -- BG & Window palette data (r/w)
        //OBP0 -- Object Palette 0 data (r/w)
        //OBP1 -- Object Palette 1 data (r/w)
        return memory[offsetAddr];
    case 0xFF4A: case 0xFF4B:
        //Window x and y positions
        return memory[offsetAddr];
    
    default:
        std::cout << std::endl;
        throw std::runtime_error("Bad IO read address");
    }
}

void IO_Manager::incrementTimer()
{
    /*
    Increments and resets the current timer
    If timer overflows, an interrupt is triggered
    */
    tCycleCount = 0;
    // Inc counter and detect overflow
    if(++memory[T_COUNTER] == 0)
    {
        memory[T_COUNTER] = memory[T_MODULO];
        memory[INTERRUPTS] |= TIMER_INTERRUPT;
    }
}

void IO_Manager::updateTimers(uint64_t cycle)
{
    uint64_t dt = cycle-lastCycle;
    lastCycle = cycle;

    //Inc timer by real cycle time
    vCycleCount += dt;
    tCycleCount += dt;
    //Timer increments every 64 cycles
    memory[DIV_TIMER] = (cycle / 64) % 0x100;
    switch(memory[T_CONTROL]&0x07)
    {
        case 0x04:
            //4096 Hz
            if(tCycleCount >= 1024) incrementTimer();
            break;
        case 0x05:
            //262144 Hz
            if(tCycleCount >= 16) incrementTimer();
            break;
        case 0x06:
            //65536 Hz
            if(tCycleCount >= 64) incrementTimer();
            break;
        case 0x07:
            // 16384 Hz
            if(tCycleCount >= 256) incrementTimer();
            break;
        default:
            //Timer is disabled
            break;
    }
}

bool IO_Manager::spriteOverridesPixel(int screenX, int screenY, uint8_t &color) const
{
    /*
    Uses the sprite attributes and pattern data to set &color
    If this color is definitely in the forground, returns true.
    If no color can be found or the color is transparent, color is not modified
    */

    // Set to 8 of 16 height mode
    uint8_t height = 8 + 2*(memory[LCDC]&0x04);
    // Sprites are enabled, draw them
    for(const SpriteAttribute &attribs : sprites)
    {
        // TODO: maybe implement sprite count bug?
        // Check if sprite contains x coord
        if(attribs[1]-8 > screenX || attribs[1] <= screenX) continue;
        if(attribs[0]-16 > screenY || (attribs[0]+height)-16 <= screenY) continue;

        // Pixel is definitely in current sprite
        // Get relative tile coord
        uint8_t tileX = (8+screenX) - attribs[1];
        uint8_t tileY = (16+screenY)- attribs[0];

        // Mirror patterns if attrib is set
        if((attribs[3]&0x20)) tileX = 8-tileX;
        if((attribs[3]&0x40)) tileY = height-tileY;

        // Gets the pattern table index of the current tile
        uint8_t tileIndex = attribs[2];
        if((memory[LCDC]&0x04))
        {
            // 16px height -- ignore lower bit
            tileIndex = (attribs[2]&0x02)+(tileY>7);
        }

        // Extract color from tile and color palette
        const Tile &tile = patternTables[tileIndex];
        bool upper = (tile[tileY%8][0]>>(7-tileX)) & 1;
        bool lower = (tile[tileY%8][1]>>(7-tileX)) & 1;
        uint8_t colorIndex = (upper<<1) + lower;

        if(colorIndex == 0) return false; // Sprite at this location is transparent

        // Select the color palette
        uint8_t colorPalette = memory[O0_Palette+(bool)(attribs[3]&0x10)];

        // Sets color reference and returns
        color = (colorPalette>>(2*colorIndex))&0x3;

        // First sprite has draw priority so return immediately
        return !(attribs[3]&0x80); // Attrib 7 determines forground priority
    }
    return false;
}

void IO_Manager::backgroundPixel(int screenX, int screenY, uint8_t &color) const
{
    /*
    Uses the background map and pattern data to set &color
    If background color is zero, color is not modified
    */
    int bgX = screenX + memory[GB_SCX];
    int bgY = screenY + memory[GB_SCY];

    // Read tile index from correct background map
    uint16_t tileIndex = backgroundMap1[(bgY/8)%0x20][(bgX/8)%0x20];
    if((memory[LCDC]&0x08)) tileIndex = backgroundMap2[(bgY/8)%0x20][(bgX/8)%0x20];

    // Correct index using the signed lookup table if requested
    if(!(memory[LCDC]&0x10))
    {
        int8_t indexOffset = *reinterpret_cast<int8_t *>(&tileIndex);
        tileIndex = 0x100+indexOffset;
    }

    const Tile &tile = patternTables[tileIndex];
    uint8_t upper = (tile[bgY%8][0]>>(7-bgX%8)) & 1;
    uint8_t lower = (tile[bgY%8][1]>>(7-bgX%8)) & 1;
    uint8_t colorIndex = (upper<<1) + lower;

    // Gets the true background color
    uint8_t bgColor = (memory[BG_Palette]>>(2*colorIndex))&0x3;
    if(bgColor!=0)  color = bgColor;
}

void IO_Manager::drawLine() const
{
    /*
    Draws the current line (index 0xFF44) onto the display
    This draws: background, window, sprites
    */
    int screenY = memory[LCD_LY];
    for(int screenX=0; screenX<SCREEN_WIDTH; screenX++)
    {
        uint8_t pixelColor = 0;
        if((memory[LCDC]&0x02) && spriteOverridesPixel(screenX, screenY, pixelColor))
        {
            drawPixel(pixelColor, screenX, screenY);
            continue;
        }
        if((memory[LCDC]&0x01))
        {
            backgroundPixel(screenX, screenY, pixelColor);
        }

        drawPixel(pixelColor, screenX, screenY);
    }
}

void IO_Manager::updateLCD()
{
    // Timings from http://bgb.bircd.org/pandocs.htm#videodisplay
    switch(vCycleCount)
    {
        case 0 ... 65663:
            // Drawing to screen
            switch (vCycleCount%456)
            {
            case 0 ... 77:
                // Mode 2: (don't need to emulate OAM)
                // If H-Blank was successful, start rendering next line
                if((memory[LCD_STAT]&0x03) != 0x02)
                {
                    // New row has started, increment counter
                    memory[LCD_LY]++;
                    // Trigger interrupt if enabled
                    if((memory[LCD_STAT]&0x20))
                    {   // Could convert this to single bitwise
                        memory[INTERRUPTS] |= STAT_INTERRUPT;
                    }
                }
                memory[LCD_STAT] = (memory[LCD_STAT]&0xF8) | 0x02;
                break;
            case 78 ... 246:
                // Mode 3: (don't need to emulate OAM)
                memory[LCD_STAT] = (memory[LCD_STAT]&0xF8) | 0x03;
                break;
            default:
                // Mode 0: scan line needs to be drawn
                // Only attempt draw once per line
                if((memory[LCD_STAT]&0x03) != 0x00)
                {
                    drawLine();
                    // Trigger interrupt if enabled
                    if((memory[LCD_STAT]&0x08))
                    {
                        memory[INTERRUPTS] |= STAT_INTERRUPT;
                    }
                }
                memory[LCD_STAT] = (memory[LCD_STAT]&0xF8);
                break;
            }
            break;
        case 65664 ... 70223:
            // Mode 1: VBlank period
            if((memory[LCD_STAT]&0x03) != 0x01)
            {
                // Trigger vsync interrupt always
                memory[INTERRUPTS] |= VSYNC_INTERRUPT;

                // Trigger stat interrupt if enabled
                if((memory[LCD_STAT]&0x10))
                {
                    memory[INTERRUPTS] |= STAT_INTERRUPT;
                }
            }
            memory[LCD_STAT] = (memory[LCD_STAT]&0xF8) | 0x01;
            break;
        default:
            // VBlank finished... flush screen
            vCycleCount = 0;
            memory[LCD_LY] = 255;
            finishRender();
            break;
    }

    memory[LCD_STAT] |= (memory[LCD_LY] == memory[LCD_LYC]) << 2;
    if((memory[LCD_STAT]&0x40) && (memory[LCD_STAT]&0x04))
    {
        memory[INTERRUPTS] |= STAT_INTERRUPT;
    }
}

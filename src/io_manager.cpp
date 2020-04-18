#include "io_manager.hpp"

#include <iostream>

IO_Manager::IO_Manager()
{
    memory.fill(0); //For debuging
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
    switch (addr)
    {
    case 0xFF04:
        //DIV -- Divider Register (cannot write data)
        throw std::runtime_error("DIV write unsupported");
        break;
    case 0xFF02:
        //SC -- SIO control (r/w)
        // Immediately display serial data to console
        //std::cout<<memory[SERIAL_DATA];
        memory[addr-IO_OFFSET] = value;
        break;
    case 0xFF41:
        // LCD Status Register
        memory[LCD_STAT] = 0x80|(memory[LCD_STAT]&0x07)|(value&0x78);
        break;
    case 0xFF44:
        //LY -- Scroll Y (r)
        throw std::runtime_error("Cannot write to LY @ 0xFF44");
    case 0xFF4B:
        //std::cout << "Write to window x: " << (int)value << std::endl;
        memory[addr-IO_OFFSET] = value;
        break;
    case 0xFF00:
        //P1 Joypad
        memory[addr-IO_OFFSET] = 0xC0|(value&0x30);
        break;
    case 0xFF05:
        // Should only update timers between instructions when accessed
        updateTimers();
    default:
        // Most IO actions don't require immediate action, deal with it later
        memory[addr-IO_OFFSET] = value;
        break;
    }
}

uint8_t IO_Manager::ioRead(uint16_t addr)
{
    switch(addr)
    {
    case 0xFF00: {
        // Joypad TODO: keypresses
        uint8_t value = 0xFF;
        if(!(memory[addr-IO_OFFSET]&0x10))
        {
            // P14 select and any p14 pressed keys are 0
            value &= 0xE0|(inputs&0x0F);
        }
        if(!(memory[addr-IO_OFFSET]&0x20))
        {
            // P15 select and any p15 pressed keys are 0
            value &= 0xD0|(inputs>>4);
        }
        return value;
    }
    case 0xFF46:
        // DMA reads are never allowed
        throw std::runtime_error("DMA read not permitted!");

    //Sound
    case 0xFF10:
        return 0x80|memory[addr-IO_OFFSET];
    case 0xFF11:
        return 0x3F|memory[addr-IO_OFFSET];
    case 0xFF13: case 0xFF18: case 0xFF1D:
        return 0xFF;
    case 0xFF14: case 0xFF19: case 0xFF1E:
        return 0xBF|memory[addr-IO_OFFSET];
    case 0xFF16:
        return 0x3F|memory[addr-IO_OFFSET];
    case 0xFF1A:
        return 0x7F|memory[addr-IO_OFFSET];
    case 0xFF1C:
        return 0xF9|memory[addr-IO_OFFSET];
    case 0xFF20:
        return 0xC0|memory[addr-IO_OFFSET];
    case 0xFF23:
        return 0xBF|memory[addr-IO_OFFSET];
    case 0xFF26:
        std::cout << "Read NR52 requested" << std::endl;
        throw std::runtime_error("Read NR52 requested");
    //Video
    case 0xFF04: case 0xFF05:
        // Timers lazy update
        // Should only update timers between instructions when accessed
        updateTimers();
    default:
        return memory[addr-IO_OFFSET];
    }
}

void IO_Manager::reduceTimer(uint_fast16_t threshold)
{
    /*
    Reduces the tCycleCount to within 'threshold' by incrementing the timer
    If timer overflows, an interrupt is triggered
    */
    while(tCycleCount >= threshold)
    {
        tCycleCount -= threshold;

        // Inc counter and detect overflow
        if(((++memory[T_COUNTER])&0xFF) == 0)
        {
            memory[T_COUNTER] = memory[T_MODULO];
            memory[INTERRUPTS] |= TIMER_INTERRUPT;
        }
    }
}

void IO_Manager::updateTimers()
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
            reduceTimer(256);
            break;
        case 0x05:
            //262144 Hz
            reduceTimer(4);
            break;
        case 0x06:
            //65536 Hz
            reduceTimer(16);
            break;
        case 0x07:
            // 16384 Hz
            reduceTimer(64);
            break;
        default:
            //Timer is disabled
            break;
    }
}

void IO_Manager::pressKey(Key key)
{
    /*
    Presses key and triggers interrupt.
    Only call once per keypress if interrupts are important
    */
    memory[INTERRUPTS] |= INPUT_INTERRUPT;
    inputs &= ~static_cast<uint8_t>(key);
}

void IO_Manager::releaseKey(Key key)
{
    /*
    Unpresses key, no interrupt is triggered
    */
    inputs |= static_cast<uint8_t>(key);
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
        if((attribs[3]&0x20)) tileX = 7-tileX;
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

        if(colorIndex == 0) continue; // Sprite at this location is transparent

        // Select the color palette
        uint8_t colorPalette = memory[O0_Palette+(bool)(attribs[3]&0x10)];

        // Sets color reference and returns
        color = (colorPalette>>(2*colorIndex))&0x3;

        // First sprite has draw priority so return immediately
        return !(attribs[3]&0x80); // Attrib 7 determines forground priority
    }
    return false;
}

uint8_t IO_Manager::pixelFromMap(uint16_t mapX, uint16_t mapY, bool map2) const
{
    /*
    Returns the pixel located at the corresponding map coordinates
    When map2 == False: backgroundMap1 is used for tile resolution
    When map2 == True: backgroundMap2 is used for tile resolution
    */

    // Read tile index from correct background map
    uint16_t tileIndex = backgroundMap1[(mapY/8)%0x20][(mapX/8)%0x20];
    if(map2) tileIndex = backgroundMap2[(mapY/8)%0x20][(mapX/8)%0x20];

    // Correct index using the signed lookup table if requested
    if(!(memory[LCDC]&0x10))
    {
        int8_t indexOffset = *reinterpret_cast<int8_t *>(&tileIndex);
        tileIndex = 0x100+indexOffset;
    }

    const Tile &tile = patternTables[tileIndex];
    uint8_t upper = (tile[mapY%8][0]>>(7-mapX%8)) & 1;
    uint8_t lower = (tile[mapY%8][1]>>(7-mapX%8)) & 1;
    uint8_t colorIndex = (upper<<1) + lower;

    // Gets the true background color
    return (memory[BG_Palette]>>(2*colorIndex))&0x3;
}

void IO_Manager::backgroundPixel(int screenX, int screenY, uint8_t &color) const
{
    /*
    Uses the background map and pattern data to set &color
    If background color is zero, color is not modified
    */
    int bgX = screenX + memory[BG_SCX];
    int bgY = screenY + memory[BG_SCY];

    uint8_t bgColor = pixelFromMap(bgX, bgY, memory[LCDC]&0x08);
    if(bgColor!=0)  color = bgColor;
}

bool IO_Manager::windowOverridesPixel(int screenX, int screenY, uint8_t &color) const
{
    /*
    Uses the window map and pattern data to set &color
    Window is always drawn on top of background
    */
    int windowX = screenX-memory[WINDOW_X]+7;
    int windowY = windowOffsetY- memory[WINDOW_Y];

    if(memory[WINDOW_X]>166)
    {
        return false;
    }
    if(windowX>=0 && windowX<160 && windowY>=0 && windowY<144)
    {
        color = pixelFromMap(windowX, windowY, memory[LCDC]&0x40);
        return true;
    }
    return false;
}

void IO_Manager::drawLine() const
{
    /*
    Draws the current line (index 0xFF44) onto the display
    This draws: background, window, sprites
    */
    int screenY = memory[LCD_LY];
    if(memory[WINDOW_X]<=166 || memory[LCDC]&0x20)
    {
        (*const_cast<uint32_t*>(&windowOffsetY))++;
    }
    for(int screenX=0; screenX<SCREEN_WIDTH; screenX++)
    {
        uint8_t pixelColor = 0;
        if((memory[LCDC]&0x02) && spriteOverridesPixel(screenX, screenY, pixelColor))
        {
            drawPixel(pixelColor, screenX, screenY);
            continue;
        }
        if((memory[LCDC]&0x20) && windowOverridesPixel(screenX, screenY, pixelColor))
        {
            drawPixel(pixelColor, screenX, screenY);
            continue;
        }
        if((memory[LCDC]&0x01))
            backgroundPixel(screenX, screenY, pixelColor);
        

        drawPixel(pixelColor, screenX, screenY);
    }
}

bool IO_Manager::setLCDStage(uint8_t stage, bool interrupt)
{
    /*
    Sets the first 2 bits of the LCD_STAT register to the selected stage.
    Returns true if a new stage is entered
    If enabled and a new stage is entered, trigger the interrupt flag.
    */
    if((memory[LCD_STAT]&0x03) != stage)
    {
        // Trigger interrupt if stage changed and interrupt enabled
        if(interrupt)   memory[INTERRUPTS] |= STAT_INTERRUPT;

        memory[LCD_STAT] = (memory[LCD_STAT]&0xF8) | stage;
        return true;
    }
    return false;
}

void IO_Manager::updateLCD()
{
    // Timings from http://bgb.bircd.org/pandocs.htm#videodisplay
    // y-scan should increment throughout the entire draw process
    memory[LCD_LY] = vCycleCount/456;
    switch(vCycleCount)
    {
        case 0 ... 65663:
            // Drawing to screen
            switch (vCycleCount%456)
            {
            case 0 ... 77:
                // Mode 2: (don't need to emulate OAM)
                // Needs to trigger interrupt if enabled
                setLCDStage(0x02, memory[LCD_STAT]&0x20);
                break;
            case 78 ... 246:
                // Mode 3: (don't need to emulate OAM)
                memory[LCD_STAT] = (memory[LCD_STAT]&0xF8) | 0x03;
                break;
            default:
                // Mode 0: scan line needs to be drawn
                if(setLCDStage(0x00, memory[LCD_STAT]&0x08))
                {
                    // Only attempt draw once per line
                    drawLine();
                }
                break;
            }
            break;
        case 65664 ... 70223:
            // Mode 1: VBlank period
            if(setLCDStage(0x01, memory[LCD_STAT]&0x10))
            {
                // Always trigger vsync interrupt
                memory[INTERRUPTS] |= VSYNC_INTERRUPT;
            }
            break;
        default:
            // VBlank finished... flush screen
            vCycleCount = 0;
            windowOffsetY = 0;
            pollEvents();
            finishRender();
            break;
    }

    // Set compare register and trigger interrupts if needed
    memory[LCD_STAT] = (memory[LCD_STAT]&0xFB) | ((memory[LCD_LY] == memory[LCD_LYC]) << 2);
    if((memory[LCD_STAT]&0x40) && (memory[LCD_STAT]&0x04))
    {
        memory[INTERRUPTS] |= STAT_INTERRUPT;
    }
}

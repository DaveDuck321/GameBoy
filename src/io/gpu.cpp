#include "io_manager.hpp"
#include <iostream>

bool IO_Manager::spriteOverridesPixel(int screenX, int screenY, uint8_t &color) const
{
    /*
    Uses the sprite attributes and pattern data to set &color
    If this color is definitely in the forground, returns true.
    If no color can be found or the color is transparent, color is not modified
    */
    // Set to 8 of 16 height mode
    uint8_t height = 8 + ((memory[LCDC]&0x04)<<1);
    // Sprites are enabled, draw them
    for(const SpriteAttribute &attribs : sprites)
    {
        // TODO: maybe implement sprite count bug?
        // Check if sprite contains x coord
        if((attribs.x > screenX+8) || (attribs.x <= screenX)) continue;
        if((attribs.y > screenY+16) || (attribs.y+height <= screenY+16)) continue;

        // Pixel is definitely in current sprite
        // Get relative tile coord
        uint8_t tileX = (8 + screenX) - attribs.x;
        uint8_t tileY = (16 + screenY) - attribs.y;

        // Mirror patterns if attrib is set
        if((attribs.attribs&0x20)) tileX = 7-tileX;
        if((attribs.attribs&0x40)) tileY = height-tileY;

        // Gets the pattern table index of the current tile
        uint8_t tileIndex = attribs.tile;
        if((memory[LCDC]&0x04))
        {
            // 16px height -- ignore lower bit
            tileIndex = (attribs.tile&0xFE)+(tileY>7);
        }

        // Extract color from tile and color palette
        const Tile &tile = patternTables[tileIndex];
        bool upper = (tile[tileY%8][1]>>(7-tileX)) & 1;
        bool lower = (tile[tileY%8][0]>>(7-tileX)) & 1;
        uint8_t colorIndex = (upper<<1) | lower;

        if(colorIndex == 0) continue; // Sprite at this location is transparent

        // Select the color palette
        uint8_t colorPalette = memory[O0_Palette+((attribs.attribs&0x10)>>4)];

        // Sets color reference and returns
        color = (colorPalette>>(2*colorIndex))&0x03;

        // First sprite has draw priority so return immediately
        return !(attribs.attribs&0x80); // Attrib 7 determines forground priority
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
    uint8_t upper = (tile[mapY%8][1]>>(7-mapX%8)) & 1;
    uint8_t lower = (tile[mapY%8][0]>>(7-mapX%8)) & 1;
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

        memory[LCD_STAT] = (memory[LCD_STAT]&0xFC) | stage;
        return true;
    }
    return false;
}

void IO_Manager::updateLCD()
{
    // Timings from http://bgb.bircd.org/pandocs.htm#videodisplay
    // y-scan should increment throughout the entire draw process
    if(!(memory[LCDC]&0x80))
    {
        vCycleCount = 0;
        memory[LCD_LY] = 0;
        memory[LCD_STAT] = (memory[LCD_STAT]&0xFC);
        return;
    }
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
                if(setLCDStage(0x02, memory[LCD_STAT]&0x20))
                {
                    // Set compare register and trigger interrupts if needed
                    memory[LCD_STAT] = (memory[LCD_STAT]&0xFB) | ((memory[LCD_LY] == memory[LCD_LYC]) << 2);
                    if((memory[LCD_STAT]&0x40) && (memory[LCD_STAT]&0x04))
                        memory[INTERRUPTS] |= STAT_INTERRUPT;
                }
                break;
            case 78 ... 246:
                // Mode 3: (don't need to emulate OAM)
                memory[LCD_STAT] = (memory[LCD_STAT]&0xFC) | 0x03;
                break;
            default:
                // Mode 0: scan line needs to be drawn
                if(setLCDStage(0x00, memory[LCD_STAT]&0x08))
                {
                    // Only attempt draw once per line
                    // Only draw when frame requested
                    if(frameScheduled)  drawLine();
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
            finishRender(); // VBlank finished... flush screen
            pollEvents(); // Render started, calculate frameskip, get inputs
            // Reset registers
            memory[LCD_LY] = 0;
            vCycleCount = 0;
            windowOffsetY = 0;
            break;
    }
}

#include <stdlib.h>
#include <string.h>

#include "pico/types.h"
#include "gfxcanvas.hpp"


GFXcanvas16::GFXcanvas16 (uint16_t w, uint16_t h) : Adafruit_GFX(w, h) 
{
    uint32_t bytes = w * h * 2;
    if ((buffer = (uint16_t *)malloc(bytes))) {
        memset(buffer, 0, bytes);
    }
}

GFXcanvas16::~GFXcanvas16(void) 
{
    if (buffer)
        free(buffer);
}

void GFXcanvas16::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
    if (!buffer) 
        return;
        
        
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
        return;

    int16_t t;
    switch (_rotation) {
        case 1:
            t = x;
            x = WIDTH - 1 - y;
            y = t;
            break;
        case 2:
            x = WIDTH - 1 - x;
            y = HEIGHT - 1 - y;
            break;
        case 3:
            t = x;
            x = y;
            y = HEIGHT - 1 - t;
            break;
    }

    buffer[x + y * WIDTH] = color;
}

uint16_t GFXcanvas16::getPixel(int16_t x, int16_t y) const 
{
    int16_t t;
    switch (_rotation) {
        case 1:
            t = x;
            x = WIDTH - 1 - y;
            y = t;
            break;
        case 2:
            x = WIDTH - 1 - x;
            y = HEIGHT - 1 - y;
            break;
        case 3:
            t = x;
            x = y;
            y = HEIGHT - 1 - t;
            break;
    }
    return getRawPixel(x, y);
}

uint16_t GFXcanvas16::getRawPixel(int16_t x, int16_t y) const 
{
    if ((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT))
        return 0;

    if (buffer)
        return buffer[x + y * WIDTH];
    
    return 0;
}

void GFXcanvas16::fillScreen(uint16_t color) 
{
    if (!buffer)
        return;

    uint8_t hi = color >> 8, lo = color & 0xFF;
    if (hi == lo) {
        memset(buffer, lo, WIDTH * HEIGHT * 2);
    }
    else {
        uint32_t i, pixels = WIDTH * HEIGHT;
        for (i = 0; i < pixels; i++)
            buffer[i] = color;
    }
}

/*void GFXcanvas16::byteSwap(void) {
    if (!buffer)
        return;

    uint32_t i, pixels = WIDTH * HEIGHT;
    for (i = 0; i < pixels; i++)
      buffer[i] = __builtin_bswap16(buffer[i]);
}*/

void GFXcanvas16::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (h < 0) { // Convert negative heights to positive equivalent
        h *= -1;
        y -= h - 1;
        if (y < 0) {
            h += y;
            y = 0;
        }
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((x < 0) || (x >= width()) || (y >= height()) || ((y + h - 1) < 0)) {
        return;
    }

    if (y < 0) { // Clip top
        h += y;
        y = 0;
    }
    if (y + h > height()) { // Clip bottom
        h = height() - y;
    }

    if (getRotation() == 0) {
        drawFastRawVLine(x, y, h, color);
    } 
    else if (getRotation() == 1) {
        int16_t t = x;
        x = WIDTH - 1 - y;
        y = t;
        x -= h - 1;
        drawFastRawHLine(x, y, h, color);
    } 
    else if (getRotation() == 2) {
        x = WIDTH - 1 - x;
        y = HEIGHT - 1 - y;

        y -= h - 1;
        drawFastRawVLine(x, y, h, color);
    } 
    else if (getRotation() == 3) {
        int16_t t = x;
        x = y;
        y = HEIGHT - 1 - t;
        drawFastRawHLine(x, y, h, color);
    }
}

void GFXcanvas16::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (w < 0) { // Convert negative widths to positive equivalent
        w *= -1;
        x -= w - 1;
        if (x < 0) {
            w += x;
            x = 0;
        }
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((y < 0) || (y >= height()) || (x >= width()) || ((x + w - 1) < 0)) {
        return;
    }

    if (x < 0) { // Clip left
        w += x;
        x = 0;
    }
    if (x + w >= width()) { // Clip right
        w = width() - x;
    }

    if (getRotation() == 0) {
        drawFastRawHLine(x, y, w, color);
    } 
    else if (getRotation() == 1) {
        int16_t t = x;
        x = WIDTH - 1 - y;
        y = t;
        drawFastRawVLine(x, y, w, color);
    } 
    else if (getRotation() == 2) {
        x = WIDTH - 1 - x;
        y = HEIGHT - 1 - y;

        x -= w - 1;
        drawFastRawHLine(x, y, w, color);
    } 
    else if (getRotation() == 3) {
        int16_t t = x;
        x = y;
        y = HEIGHT - 1 - t;
        y -= w - 1;
        drawFastRawVLine(x, y, w, color);
    }
}

void GFXcanvas16::drawFastRawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    // x & y already in raw (rotation 0) coordinates, no need to transform.
    uint16_t *buffer_ptr = buffer + y * WIDTH + x;
    for (int16_t i = 0; i < h; i++) {
        (*buffer_ptr) = color;
        buffer_ptr += WIDTH;
    }
}

void GFXcanvas16::drawFastRawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    // x & y already in raw (rotation 0) coordinates, no need to transform.
    uint32_t buffer_index = y * WIDTH + x;
    for (uint32_t i = buffer_index; i < buffer_index + w; i++) {
        buffer[i] = color;
    }
}
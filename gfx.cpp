#include "gfx.hpp"
#include "gfxfont.cpp"
#include <math.h>

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)     \
    {                           \
        int16_t t = a;          \
        a = b;                  \
        b = t;                  \
    }
#endif

Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h) : WIDTH(w), HEIGHT(h)
{
    _width = w;
    _height = h;
    _rotation = 0;
    _cursor_x = _cursor_y = 0;
    _textsize_x = _textsize_y = 1;
    _textcolor = 0xFFFF;
    _gfxFont = NULL;
}

void Adafruit_GFX::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } 
    else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else 
        {
            drawPixel(x0, y0, color);
        }

        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void Adafruit_GFX::setRotation(uint8_t x) 
{
    _rotation = (x & 3);

    switch (_rotation) 
    {
        case 0:
        case 2:
            _width = WIDTH;
            _height = HEIGHT;
            break;
        case 1:
        case 3:
            _width = HEIGHT;
            _height = WIDTH;
            break;
    }
}

void Adafruit_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
    writeLine (x, y, x, y + h - 1, color);
}

void Adafruit_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    writeLine (x, y, x + w - 1, y, color);
}

void Adafruit_GFX::drawLine (int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
    // Update in subclasses if desired!
    if (x0 == x1) 
    {
        if (y0 > y1)
            _swap_int16_t(y0, y1);

        drawFastVLine(x0, y0, y1 - y0 + 1, color);
    } 
    else if (y0 == y1) {
        if (x0 > x1)
            _swap_int16_t(x0, x1);
        drawFastHLine(x0, y0, x1 - x0 + 1, color);
    } 
    else {
        writeLine(x0, y0, x1, y1, color);
    }
}

void Adafruit_GFX::drawRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    for (int16_t i = x; i < x + w; i++) {
        drawFastVLine(i, y, h, color);
    }
}


void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void Adafruit_GFX::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) 
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}

void Adafruit_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Adafruit_GFX::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) 
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    delta++; // Avoid some +1's in the loop

    while (x < y) 
    {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;
        
        // These checks avoid double-drawing certain lines, important
        // for the SSD1306 library which has an INVERT drawing mode.
        if (x < (y + 1)) {
            if (corners & 1)
                drawFastVLine(x0 + x, y0 - y, 2 * y + delta, color);

            if (corners & 2)
                drawFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
        }
        if (y != py) {
            if (corners & 1)
                drawFastVLine(x0 + py, y0 - px, 2 * px + delta, color);

            if (corners & 2)
                drawFastVLine(x0 - py, y0 - px, 2 * px + delta, color);

            py = y;
        }
        px = x;
    }
}

void Adafruit_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) 
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;

    // smarter version
    drawFastHLine(x + r, y, w - 2 * r, color);         // Top
    drawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    drawFastVLine(x, y + r, h - 2 * r, color);         // Left
    drawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right

    // draw four corners
    drawCircleHelper(x + r, y + r, r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

void Adafruit_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) 
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;

    // smarter version
    fillRect(x + r, y, w - 2 * r, h, color);

    // draw four corners
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void Adafruit_GFX::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void Adafruit_GFX::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;

        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;

        drawFastHLine(a, y0, b - a + 1, color);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0;
    int16_t dx02 = x2 - x0, dy02 = y2 - y0;
    int16_t dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1; // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_int16_t(a, b);

        drawFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_int16_t(a, b);
        drawFastHLine(a, y, b - a + 1, color);
    }
}

/* ------------- text handling ----------- */

void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
{
  drawChar(x, y, c, color, bg, size, size);
}

void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) 
{
    if (!_gfxFont)  // 'Classic' built-in font
    { 
        if ((x >= _width) ||              // Clip right
            (y >= _height) ||             // Clip bottom
            ((x + 6 * size_x - 1) < 0) || // Clip left
            ((y + 8 * size_y - 1) < 0))   // Clip top
            return;

        if (/*!_cp437 &&*/ (c >= 176))
            c++; // Handle 'classic' charset behavior

        for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
            uint8_t line = default_font[c * 5 + i];
            for (int8_t j = 0; j < 8; j++, line >>= 1) {
                if (line & 1) {
                    if (size_x == 1 && size_y == 1)
                        drawPixel(x + i, y + j, color);
                    else
                        fillRect(x + i * size_x, y + j * size_y, size_x, size_y, color);
                } 
                else if (bg != color) {
                    if (size_x == 1 && size_y == 1)
                        drawPixel(x + i, y + j, bg);
                    else
                        fillRect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
                }
            }
        }

        if (bg != color) { // If opaque, draw vertical line for last column
            if (size_x == 1 && size_y == 1)
                drawFastVLine(x + 5, y, 8, bg);
            else
                fillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
        }
    }
    else { // Custom font
        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!
        c -= (uint8_t)_gfxFont->first;
        GFXglyph *glyph = _gfxFont->glyph + c;
        uint8_t *bitmap = _gfxFont->bitmap;

        uint16_t bo = glyph->bitmapOffset;
        uint8_t w = glyph->width;
        uint8_t h = glyph->height;
        int8_t xo = glyph->xOffset;
        int8_t yo = glyph->yOffset;
        uint8_t xx, yy, bits = 0, bit = 0;
        int16_t xo16 = 0, yo16 = 0;

        if (size_x > 1 || size_y > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.

        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (!(bit++ & 7)) {
                    bits = bitmap[bo++];
                }
                if (bits & 0x80) {
                    if (size_x == 1 && size_y == 1) {
                        drawPixel(x + xo + xx, y + yo + yy, color);
                    } else {
                        fillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y, size_x, size_y, color);
                    }
                }
                bits <<= 1;
            }
        }
    } // End classic vs custom font
}

size_t Adafruit_GFX::write(uint8_t c) 
{
    if (!_gfxFont) { // 'Classic' built-in font
        if (c == '\n') {              // Newline?
            _cursor_x = 0;                // Reset x to zero,
            _cursor_y += _textsize_y * 8; // advance y one line
        } 
        else if (c != '\r') {       // Ignore carriage returns
            if (_text_wrap && ((_cursor_x + _textsize_x * 6) > _width)) { // Off right?
                _cursor_x = 0;                                       // Reset x to zero,
                _cursor_y += _textsize_y * 8; // advance y one line
            }

            // TODO: Consider background color support
            drawChar (_cursor_x, _cursor_y, c, _textcolor, 0x0000, _textsize_x, _textsize_y);
            _cursor_x += _textsize_x * 6; // Advance x one char
        }
    }
    else { // Custom font
        if (c == '\n') {
            _cursor_x = 0;
            _cursor_y += _textsize_y * _gfxFont->yAdvance;
        } 
        else if (c != '\r') {
            uint8_t first = _gfxFont->first;
            if ((c >= first) && (c <= _gfxFont->last)) {
                GFXglyph *glyph = _gfxFont->glyph + (c - first);
                uint8_t w = glyph->width;
                uint8_t h = glyph->height;

                if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
                    int16_t xo = glyph->xOffset; // sic
                    if (_text_wrap && ((_cursor_x + _textsize_x * (xo + w)) > _width)) {
                        _cursor_x = 0;
                        _cursor_y += _textsize_y * _gfxFont->yAdvance;
                    }
                
                    drawChar(_cursor_x, _cursor_y, c, _textcolor, 0x0000, _textsize_x, _textsize_y);
                }
                
                _cursor_x += glyph->xAdvance * _textsize_x;
            }
        }
    }
    return 1;
}


void Adafruit_GFX::setTextSize(uint8_t s_x, uint8_t s_y) 
{
    _textsize_x = (s_x > 0) ? s_x : 1;
    _textsize_y = (s_y > 0) ? s_y : 1;
}
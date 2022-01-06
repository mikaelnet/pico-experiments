#ifndef _ADAFRUIT_GFX_HPP
#define _ADAFRUIT_GFX_HPP

#include "pico/stdlib.h"
#include "gfxfont.hpp"

class Adafruit_GFX 
{
    public:
        Adafruit_GFX(int16_t w, int16_t h);

        virtual void setRotation(uint8_t r);

        virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

        virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

        virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        virtual void fillScreen(uint16_t color);
        // Optional and probably not necessary to change
        virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

        void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
        void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
        void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
        void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
        void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
        void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
        void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
        void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

        void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
        void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bgColor);

        int16_t width(void) const { return _width; }
        int16_t height(void) const { return _height; }
        uint8_t getRotation(void) const { return _rotation; }

        // Text handling
        virtual size_t write(uint8_t);

        void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
        void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);

        void setCursor(int16_t x, int16_t y) { _cursor_x = x; _cursor_y = y; }
        void setTextColor(uint16_t c) { _textcolor = c; }
        void setTextWrap(bool w) { _text_wrap = w; }
        // getTextBounds
        void setTextSize(uint8_t s) { setTextSize(s, s); };
        void setTextSize(uint8_t sx, uint8_t sy);
        void setFont(const GFXfont *f = NULL);

        int16_t getCursorX(void) const { return _cursor_x; }
        int16_t getCursorY(void) const { return _cursor_y; };


    protected:
        virtual void writeLine (int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

        int16_t WIDTH;          ///< This is the 'raw' display width - never changes
        int16_t HEIGHT;         ///< This is the 'raw' display height - never changes

        int16_t _width;         ///< Display width as modified by current rotation
        int16_t _height;        ///< Display height as modified by current rotation

        uint8_t _rotation;      ///< Display rotation (0 thru 3)

        // Text handling
        int16_t _cursor_x;      ///< x location to start print()ing text
        int16_t _cursor_y;      ///< y location to start print()ing text
        uint16_t _textcolor;    ///< 16-bit background color for print()
        uint8_t _textsize_x;    ///< Desired magnification in X-axis of text to print()
        uint8_t _textsize_y;    ///< Desired magnification in Y-axis of text to print()
        bool _text_wrap;
        GFXfont *_gfxFont; 
};

#endif // _ADAFRUIT_GFX_HPP

#ifndef _ADAFRUIT_GFXCANVAS_HPP
#define _ADAFRUIT_GFXCANVAS_HPP

#include "gfx.hpp"

class GFXcanvas16 : public Adafruit_GFX 
{
    public:
        GFXcanvas16(uint16_t w, uint16_t h);
        ~GFXcanvas16(void);
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void fillScreen(uint16_t color);
        //void byteSwap(void);
        void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        uint16_t getPixel(int16_t x, int16_t y) const;

        uint16_t *getBuffer(void) const { return buffer; }

    protected:
        uint16_t getRawPixel(int16_t x, int16_t y) const;
        void drawFastRawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void drawFastRawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

    private:
        uint16_t *buffer;
};

#endif


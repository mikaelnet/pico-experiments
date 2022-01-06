#include "display.hpp"

//Adafruit_HX8340B display(spi0, TFT_MOSI, TFT_CLK, TFT_RST, TFT_CS);
Adafruit_HX8340B tft(pio0, TFT_MOSI, TFT_CLK, TFT_RST);
GFXcanvas16 canvas1(HX8340B_LCDWIDTH, HX8340B_LCDHEIGHT);
GFXcanvas16 canvas2(HX8340B_LCDWIDTH, HX8340B_LCDHEIGHT);
GFXcanvas16 *canvas = &canvas1;

void drawtext(const char *text) 
{
    for (const char *ptr = text ; *ptr ; ptr ++)
    {
        canvas->write(*ptr);
    }
}

/********************************************/
/* Sample functions for testing the display */
/********************************************/

float pi = 3.1415926;

void testLcdPattern(void)
{
    uint32_t i,j;
    tft.fillScreen(BLACK);
    tft.setWindow(0, 0, tft.width()-1, tft.height()-1);

    for(i=0;i<64;i++)
    {
        for(j=0;j<96;j++)
        {
            if (i>55)
                tft.rawWrite(WHITE);
            else if (i>47)
                tft.rawWrite(BLUE);
            else if (i>39)
                tft.rawWrite(GREEN);
            else if (i>31)
                tft.rawWrite(CYAN);
            else if (i>23)
                tft.rawWrite(RED);
            else if (i>15)
                tft.rawWrite(MAGENTA);
            else if (i>7)
                tft.rawWrite(YELLOW);
            else 
                tft.rawWrite(BLACK);
        }
    }
}

void testlines(Adafruit_GFX *gfx, uint16_t color) 
{
    gfx->fillScreen(BLACK);
    for (int16_t x=0; x < gfx->width()-1; x+=6) {
        gfx->drawLine(0, 0, x, gfx->height()-1, color);
    }
    for (int16_t y=0; y < gfx->height()-1; y+=6) {
        gfx->drawLine(0, 0, gfx->width()-1, y, color);
    }
    sleep_ms(200);

    gfx->fillScreen(BLACK);
    for (int16_t x=0; x < gfx->width()-1; x+=6) {
        gfx->drawLine(gfx->width()-1, 0, x, gfx->height()-1, color);
    }
    for (int16_t y=0; y < tft.height()-1; y+=6) {
        gfx->drawLine(gfx->width()-1, 0, 0, y, color);
    }
    sleep_ms(200);

    gfx->fillScreen(BLACK);
    for (int16_t x=0; x < gfx->width()-1; x+=6) {
        gfx->drawLine(0, gfx->height()-1, x, 0, color);
    }
    for (int16_t y=0; y < gfx->height()-1; y+=6) {
        gfx->drawLine(0, gfx->height()-1, gfx->width()-1, y, color);
    }
    sleep_ms(200);

    gfx->fillScreen(BLACK);
    for (int16_t x=0; x < gfx->width()-1; x+=6) {
        gfx->drawLine(gfx->width()-1, gfx->height()-1, x, 0, color);
    }
    for (int16_t y=0; y < gfx->height()-1; y+=6) {
        gfx->drawLine(gfx->width()-1, gfx->height()-1, 0, y, color);
    }
    sleep_ms(200);
}

void testfastlines(Adafruit_GFX *gfx, uint16_t color1, uint16_t color2) 
{
    gfx->fillScreen(BLACK);
    for (int16_t y=0; y < gfx->height()-1; y+=5) {
        gfx->drawFastHLine(0, y, gfx->width()-1, color1);
    }
    for (int16_t x=0; x < gfx->width()-1; x+=5) {
        gfx->drawFastVLine(x, 0, gfx->height()-1, color2);
    }
}

void testdrawrects(Adafruit_GFX *gfx, uint16_t color) 
{
    gfx->fillScreen(BLACK);
    for (int16_t x=0; x < gfx->height()-1; x+=6) {
        gfx->drawRect((gfx->width()-1)/2 -x/2, (gfx->height()-1)/2 -x/2 , x, x, color);
    }
}

void testfillrects(Adafruit_GFX *gfx, uint16_t color1, uint16_t color2) 
{
    gfx->fillScreen(BLACK);
    for (int16_t x=gfx->width()-1; x > 6; x-=6) {
        gfx->fillRect((gfx->width()-1)/2 -x/2, (gfx->height()-1)/2 -x/2 , x, x, color1);
        gfx->drawRect((gfx->width()-1)/2 -x/2, (gfx->height()-1)/2 -x/2 , x, x, color2);
    }
}

void testfillcircles(Adafruit_GFX *gfx, uint8_t radius, uint16_t color) 
{
    for (uint8_t x=radius; x < gfx->width()-1; x+=radius*2) {
        for (uint8_t y=radius; y < gfx->height()-1; y+=radius*2) {
            gfx->fillCircle(x, y, radius, color);
        }
    }
}

void testdrawcircles(Adafruit_GFX *gfx, uint8_t radius, uint16_t color)
{
    for (int16_t x=0; x < gfx->width()-1+radius; x+=radius*2) {
        for (int16_t y=0; y < gfx->height()-1+radius; y+=radius*2) {
            gfx->drawCircle(x, y, radius, color);
        }
    }
}

void testroundrects(Adafruit_GFX *gfx)
{
    gfx->fillScreen(BLACK);
    int color = 100;
    int i;
    int t;

    for(t = 0 ; t <= 4; t+=1) {
        int x = 0;
        int y = 0;
        int w = gfx->width();
        int h = gfx->height();

        for(i = 0 ; i <= 24; i+=1) {
            gfx->drawRoundRect(x, y, w, h, 5, color);
            x+=2;
            y+=3;
            w-=4;
            h-=6;
            color+=1100;
        }

        color+=100;
    }
}

void testtriangles(Adafruit_GFX *gfx)
{
    gfx->fillScreen(BLACK);

    int color = 0xF800;
    int t;
    int w = gfx->width()/2;
    int x = gfx->height();
    int y = 0;
    int z = gfx->width();

    for (t = 0 ; t <= 15; t+=1) {
        gfx->drawTriangle(w, y, y, x, z, x, color);
        x-=4;
        y+=4;
        z-=4;
        color+=100;
    }
}

void testdrawtext(Adafruit_GFX *gfx)
{
    gfx->fillScreen(BLACK);
    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(0,0);

    for (uint8_t i=0; i < 168; i++) {
        if (i == '\n') 
            continue;

        gfx->write(i);
        if ((i > 0) && (i % 21 == 0)) {
            gfx->write('\n');
        }
    }
}


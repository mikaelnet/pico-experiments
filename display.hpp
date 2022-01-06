#ifndef __DISPLAY_HPP
#define __DISPLAY_HPP

#include "hardware/pwm.h"
#include "gfx.hpp"
#include "gfxcanvas.hpp"
#include "hx8340b.hpp"

#define TFT_MOSI    3
#define TFT_MISO    4
#define TFT_CLK     2
#define TFT_CS      5
#define TFT_RST     6

#define TFT_BL      7
#define TFT_BL_CHAN PWM_CHAN_B

extern Adafruit_HX8340B tft;
extern GFXcanvas16 canvas1;
extern GFXcanvas16 canvas2;
extern GFXcanvas16 *canvas;
extern void drawtext(const char *text);

/********************************************/
/* Sample functions for testing the display */
/********************************************/

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x1F00
#define	RED             0x00F8
#define	GREEN           0xE007
#define CYAN            0xFF07
#define MAGENTA         0x1FF8
#define YELLOW          0xE0FF
#define WHITE           0xFFFF

extern void testLcdPattern(void);
extern void testlines(uint16_t color);
extern void testfastlines(uint16_t color1, uint16_t color2);
extern void testdrawrects(uint16_t color);
extern void testfillrects(uint16_t color1, uint16_t color2);
extern void testfillcircles(uint8_t radius, uint16_t color);
extern void testdrawcircles(uint8_t radius, uint16_t color);
extern void testroundrects();
extern void testtriangles();
extern void testdrawtext();


#endif
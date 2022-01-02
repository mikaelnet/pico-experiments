#include "display.hpp"

Adafruit_HX8340B display(spi0, TFT_MOSI, TFT_CLK, TFT_RST, TFT_CS);

void drawtext(const char *text) 
{
    for (const char *ptr = text ; *ptr ; ptr ++)
    {
        display.write(*ptr);
    }
}

/********************************************/
/* Sample functions for testing the display */
/********************************************/

float pi = 3.1415926;

void testLcdPattern(void)
{
    uint32_t i,j;
    display.fillScreen(BLACK);
    display.setWindow(0, 0, display.width()-1, display.height()-1);

    for(i=0;i<64;i++)
    {
        for(j=0;j<96;j++)
        {
            if (i>55)
                display.rawWrite(WHITE);
            else if (i>47)
                display.rawWrite(BLUE);
            else if (i>39)
                display.rawWrite(GREEN);
            else if (i>31)
                display.rawWrite(CYAN);
            else if (i>23)
                display.rawWrite(RED);
            else if (i>15)
                display.rawWrite(MAGENTA);
            else if (i>7)
                display.rawWrite(YELLOW);
            else 
                display.rawWrite(BLACK);
        }
    }
}

void testlines(uint16_t color) 
{
    display.fillScreen(BLACK);
    for (int16_t x=0; x < display.width()-1; x+=6) {
        display.drawLine(0, 0, x, display.height()-1, color);
    }
    for (int16_t y=0; y < display.height()-1; y+=6) {
        display.drawLine(0, 0, display.width()-1, y, color);
    }
    sleep_ms(200);

    display.fillScreen(BLACK);
    for (int16_t x=0; x < display.width()-1; x+=6) {
        display.drawLine(display.width()-1, 0, x, display.height()-1, color);
    }
    for (int16_t y=0; y < display.height()-1; y+=6) {
        display.drawLine(display.width()-1, 0, 0, y, color);
    }
    sleep_ms(200);

    display.fillScreen(BLACK);
    for (int16_t x=0; x < display.width()-1; x+=6) {
        display.drawLine(0, display.height()-1, x, 0, color);
    }
    for (int16_t y=0; y < display.height()-1; y+=6) {
        display.drawLine(0, display.height()-1, display.width()-1, y, color);
    }
    sleep_ms(200);

    display.fillScreen(BLACK);
    for (int16_t x=0; x < display.width()-1; x+=6) {
        display.drawLine(display.width()-1, display.height()-1, x, 0, color);
    }
    for (int16_t y=0; y < display.height()-1; y+=6) {
        display.drawLine(display.width()-1, display.height()-1, 0, y, color);
    }
    sleep_ms(200);
}

void testfastlines(uint16_t color1, uint16_t color2) 
{
    display.fillScreen(BLACK);
    for (int16_t y=0; y < display.height()-1; y+=5) {
        display.drawFastHLine(0, y, display.width()-1, color1);
    }
    for (int16_t x=0; x < display.width()-1; x+=5) {
        display.drawFastVLine(x, 0, display.height()-1, color2);
    }
}

void testdrawrects(uint16_t color) 
{
    display.fillScreen(BLACK);
    for (int16_t x=0; x < display.height()-1; x+=6) {
        display.drawRect((display.width()-1)/2 -x/2, (display.height()-1)/2 -x/2 , x, x, color);
    }
}

void testfillrects(uint16_t color1, uint16_t color2) 
{
    display.fillScreen(BLACK);
    for (int16_t x=display.width()-1; x > 6; x-=6) {
        display.fillRect((display.width()-1)/2 -x/2, (display.height()-1)/2 -x/2 , x, x, color1);
        display.drawRect((display.width()-1)/2 -x/2, (display.height()-1)/2 -x/2 , x, x, color2);
    }
}

void testfillcircles(uint8_t radius, uint16_t color) 
{
    for (uint8_t x=radius; x < display.width()-1; x+=radius*2) {
        for (uint8_t y=radius; y < display.height()-1; y+=radius*2) {
            display.fillCircle(x, y, radius, color);
        }
    }
}

void testdrawcircles(uint8_t radius, uint16_t color)
{
    for (int16_t x=0; x < display.width()-1+radius; x+=radius*2) {
        for (int16_t y=0; y < display.height()-1+radius; y+=radius*2) {
            display.drawCircle(x, y, radius, color);
        }
    }
}

void testroundrects()
{
    display.fillScreen(BLACK);
    int color = 100;
    int i;
    int t;

    for(t = 0 ; t <= 4; t+=1) {
        int x = 0;
        int y = 0;
        int w = display.width();
        int h = display.height();

        for(i = 0 ; i <= 24; i+=1) {
            display.drawRoundRect(x, y, w, h, 5, color);
            x+=2;
            y+=3;
            w-=4;
            h-=6;
            color+=1100;
        }

        color+=100;
    }
}

void testtriangles()
{
    display.fillScreen(BLACK);

    int color = 0xF800;
    int t;
    int w = display.width()/2;
    int x = display.height();
    int y = 0;
    int z = display.width();

    for (t = 0 ; t <= 15; t+=1) {
        display.drawTriangle(w, y, y, x, z, x, color);
        x-=4;
        y+=4;
        z-=4;
        color+=100;
    }
}

void testdrawtext()
{
    display.fillScreen(BLACK);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);

    for (uint8_t i=0; i < 168; i++) {
        if (i == '\n') 
            continue;

        display.write(i);
        if ((i > 0) && (i % 21 == 0)) {
            display.write('\n');
        }
    }
}

#if false
void tftPrintTest() {
  display.fillScreen(BLACK);
  display.setCursor(0, 5);
  display.setTextColor(RED);  
  display.setTextSize(1);
  display.println("Hello World!");
  display.setTextColor(YELLOW, GREEN);
  display.setTextSize(2);
  display.println("Hello World!");
  display.setTextColor(BLUE);
  display.setTextSize(3);
  display.println(1234.567);
  delay(1500);
  display.setCursor(0, 5);
  display.fillScreen(BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(0);
  display.println("Hello World!");
  display.setTextSize(1);
  display.setTextColor(GREEN);
  display.print(pi, 5);
  display.println(" Want pi?");
  display.print(8675309, HEX); // print 8,675,309 out in HEX!
  display.println(" Print HEX");
  display.setTextColor(WHITE);
  display.println("Sketch has been running for: ");
  display.setTextColor(MAGENTA);
  display.print(millis() / 1000);
  display.setTextColor(WHITE);
  display.print(" seconds.");
}

void mediabuttons() {
 // play
  display.fillScreen(BLACK);
  display.fillRoundRect(25, 10, 78, 60, 8, WHITE);
  display.fillTriangle(42, 20, 42, 60, 90, 40, RED);
  delay(500);
  // pause
  display.fillRoundRect(25, 90, 78, 60, 8, WHITE);
  display.fillRoundRect(39, 98, 20, 45, 5, GREEN);
  display.fillRoundRect(69, 98, 20, 45, 5, GREEN);
  delay(500);
  // play color
  display.fillTriangle(42, 20, 42, 60, 90, 40, BLUE);
  delay(50);
  // pause color
  display.fillRoundRect(39, 98, 20, 45, 5, RED);
  display.fillRoundRect(69, 98, 20, 45, 5, RED);
  // play color
  display.fillTriangle(42, 20, 42, 60, 90, 40, GREEN);
}

/**************************************************************************/
/*! 
    @brief  Renders a simple test pattern on the LCD
*/
/**************************************************************************/

#endif
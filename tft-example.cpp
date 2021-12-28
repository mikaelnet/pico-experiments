#include "pico/stdlib.h"
#include "pico/types.h"
#include "pico/printf.h"
#include "hardware/adc.h"

#include "gfx.hpp"
#include "gfxcanvas.hpp"
#include "hx8340b.hpp"

#define TFT_MOSI    3
#define TFT_MISO    4
#define TFT_CLK     2
#define TFT_CS      5
#define TFT_RST     9

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

//Adafruit_HX8340B display(TFT_MOSI, TFT_CLK, TFT_RST, TFT_CS);
Adafruit_HX8340B display(spi0, TFT_MOSI, TFT_CLK, TFT_RST, TFT_CS);
//GFXcanvas16 display(HX8340B_LCDWIDTH, HX8340B_LCDHEIGHT);

float pi = 3.1415926;

void lcdTestPattern(void)
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

void drawtext(const char *text) 
{
    for (const char *ptr = text ; *ptr ; ptr ++)
    {
        display.write(*ptr);
    }
}

void setup()
{
    stdio_init_all();
    display.begin();
    display.fillScreen(BLACK);

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
}

#define CROSS_HAIR_SIZE 4
uint16_t last_X, last_Y;
void loop()
{
    adc_select_input(0);    // gpio 26, pin 31
    uint adc_x_raw = adc_read();
    adc_select_input(1);    // gpio 27, pin 32
    uint adc_y_raw = adc_read();

    const uint adc_max = (1 << 12) - 1;
    uint16_t x = adc_x_raw * display.width() / adc_max;
    uint16_t y = adc_y_raw * display.height() / adc_max;

    display.drawFastVLine(last_X, last_Y - CROSS_HAIR_SIZE, CROSS_HAIR_SIZE * 2 + 1, BLACK);
    display.drawFastHLine(last_X - CROSS_HAIR_SIZE, last_Y, CROSS_HAIR_SIZE * 2 + 1, BLACK);
    display.drawFastVLine(x, y - CROSS_HAIR_SIZE, CROSS_HAIR_SIZE * 2 + 1, 0b1111110000010000);
    display.drawFastHLine(x - CROSS_HAIR_SIZE, y, CROSS_HAIR_SIZE * 2 + 1, 0b1111110000010000);
    last_X = x;
    last_Y = y;

    char buf[20];
    snprintf (buf, sizeof(buf), "X: %4d\nY: %4d", adc_x_raw, adc_y_raw);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    drawtext(buf);
}

#if false
void loop()
{
    absolute_time_t start = get_absolute_time();
    display.fillScreen(BLACK);
    //sleep_ms(500);

    display.drawPixel (display.width()/2, display.height()/2, GREEN);
    //sleep_ms(500);

     // line draw test
    testlines(YELLOW);
    //tft_display.drawBitmap(display.getBuffer());
    //sleep_ms(500);    
    
    // optimized lines
    testfastlines(RED, BLUE);
    //sleep_ms(500);    
    
    testdrawrects(GREEN);
    //sleep_ms(1000);

    testfillrects(YELLOW, MAGENTA);
    //sleep_ms(1000);

    display.fillScreen(BLACK);
    testfillcircles(10, BLUE);
    testdrawcircles(10, WHITE);
    //sleep_ms(1000);

    testroundrects();
    //tft_display.drawBitmap(display.getBuffer());
    //sleep_ms(500);
    
    testtriangles();
    //tft_display.drawBitmap(display.getBuffer());
    //sleep_ms(500);

    //lcdTestPattern();
    //sleep_ms(500);

    testdrawtext();
    //sleep_ms(1000);

    absolute_time_t end = get_absolute_time();
    int time = absolute_time_diff_us(start, end);

    char buf[40];
    snprintf (buf, sizeof(buf), "Time:\n%d us", time);

    display.fillScreen(BLACK);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(RED);
    drawtext("Hello world!\n");
    display.setTextColor(WHITE);
    drawtext(buf);

    //tft_display.drawBitmap(display.getBuffer());

    sleep_ms(5000);
}
#endif


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

int main()
{
    setup();
    while (true)
    {
        loop();
    }
}
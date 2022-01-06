#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "spi9.pio.h"

#include "gfx.hpp"
#include "hx8340b.hpp"

#define LOW   0
#define HIGH  1

#define COMMAND 0x00
#define DATA    0x01

Adafruit_HX8340B::Adafruit_HX8340B(PIO pio, int8_t mosi_pin, int8_t clk_pin, int8_t RST) : Adafruit_GFX(HX8340B_LCDWIDTH, HX8340B_LCDHEIGHT)
{
    _pio = pio;
    _mosi_pin = mosi_pin;
    _clk_pin = clk_pin;
    _rst_pin   = RST;
}

#define DELAY_FLAG 0x80
static uint8_t initCmd[] = {
    14,                           // 14 commands in list:
    HX8340B_N_SETEXTCMD, 3      , //  1: ???, 3 args, no delay
      0xFF, 0x83, 0x40,
    HX8340B_N_SPLOUT   , DELAY_FLAG  , //  2: No args, delay follows
      150,                        //     150 ms delay
    0xCA               , 3      , //  3: Undoc'd register?  3 args, no delay
      0x70, 0x00, 0xD9,
    0xB0               , 2      , //  4: Undoc'd register?  2 args, no delay
      0x01, 0x11,
    0xC9               , 8+DELAY_FLAG, //  5: Drive ability, 8 args + delay
      0x90, 0x49, 0x10, 0x28,
      0x28, 0x10, 0x00, 0x06,
      20,                         //     20 ms delay
    HX8340B_N_SETGAMMAP, 9      , //  6: Positive gamma control, 9 args
      0x60, 0x71, 0x01,           //     2.2
      0x0E, 0x05, 0x02,
      0x09, 0x31, 0x0A,
    HX8340B_N_SETGAMMAN, 8+DELAY_FLAG, //  7: Negative gamma, 8 args + delay
      0x67, 0x30, 0x61, 0x17,     //     2.2
      0x48, 0x07, 0x05, 0x33,
      10,                         //     10 ms delay
    HX8340B_N_SETPWCTR5,       3, //  8: Power Control 5, 3 args
      0x35, 0x20, 0x45,
    HX8340B_N_SETPWCTR4, 3+DELAY_FLAG, //  9: Power control 4, 3 args + delay
      0x33, 0x25, 0x4c,
      10,                         //     10 ms delay
    HX8340B_N_COLMOD   , 1      , // 10: Color Mode, 1 arg
      0x05,                       //     0x05 = 16bpp, 0x06 = 18bpp
    HX8340B_N_DISPON   , DELAY_FLAG  , // 11: Display on, no args, w/delay
      10,                         //     10 ms delay
    HX8340B_N_CASET    , 4      , // 12: Physical column pointer, 4 args
      0x00, 0x00, 0x00, 0xaf,     //     175 (max X)
    HX8340B_N_PASET    , 4      , // 13: Physical page pointer, 4 args
      0x00, 0x00, 0x00, 0xdb,     //     219 (max Y)
    HX8340B_N_RAMWR    , 0        // 14: Start GRAM write
};

void Adafruit_HX8340B::begin()
{
    // PIN directions and default output
    gpio_init(_rst_pin);
    gpio_set_dir(_rst_pin, GPIO_OUT);
    gpio_put(_rst_pin, HIGH);

    uint offset = pio_add_program (_pio, &spi9_program);
    _stateMachine = pio_claim_unused_sm(_pio, true);
    // 7.8125 -> 16MHz
    spi9_program_init(_pio, _stateMachine, offset, 2.0f, _clk_pin, _mosi_pin);

    _dmaChannel = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(_dmaChannel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, pio_get_dreq(_pio, _stateMachine, true));
    dma_channel_configure(_dmaChannel, &c, &_pio->txf[_stateMachine], NULL, 0, false);

    // Perform a reset cycle
    reset();

    uint8_t numCommands, numArgs;
    uint16_t ms;
    uint8_t *addr = initCmd;
    numCommands = *addr ++;
    while (numCommands --)
    {
        writeCommand(*addr ++);
        numArgs = *addr ++;
        ms = numArgs & DELAY_FLAG;
        numArgs &= ~DELAY_FLAG;
        while (numArgs --) {
            writeData(*addr ++);
        }

        if (ms)
            sleep_ms(*addr ++);
    }
}

void Adafruit_HX8340B::end()
{
    dma_channel_unclaim(_dmaChannel);
}

void Adafruit_HX8340B::reset()
{
    gpio_put(_rst_pin, HIGH);
    sleep_ms(100);
    gpio_put(_rst_pin, LOW);
    sleep_ms(50);
    gpio_put(_rst_pin, HIGH);
    sleep_ms(50);
}

void Adafruit_HX8340B::writeCommand(uint8_t c) 
{
    waitUntilReady();

    //uint16_t value = c;
    //spi_write16_blocking(_spi_port, &value, 1);
    pio_sm_put_blocking(_pio, _stateMachine, COMMAND);
    pio_sm_put_blocking(_pio, _stateMachine, 1);
    pio_sm_put_blocking(_pio, _stateMachine, c);
}

void Adafruit_HX8340B::writeData(uint8_t c) 
{
    waitUntilReady();

    //uint16_t value = 0x0100 | c;
    //spi_write16_blocking(_spi_port, &value, 1);
    pio_sm_put_blocking(_pio, _stateMachine, DATA);
    pio_sm_put_blocking(_pio, _stateMachine, 1);
    pio_sm_put_blocking(_pio, _stateMachine, c);
}

void Adafruit_HX8340B::writeData16(uint16_t c) 
{
    waitUntilReady();

    //uint16_t value[2];
    //value[0] = 0x0100 | c >> 8;
    //value[1] = 0x0100 | (c & 0xFF);
    //spi_write16_blocking(_spi_port, value, 2);

    pio_sm_put_blocking(_pio, _stateMachine, DATA);
    pio_sm_put_blocking(_pio, _stateMachine, 2);
    pio_sm_put_blocking(_pio, _stateMachine, c >> 8);
    pio_sm_put_blocking(_pio, _stateMachine, c & 0xFF);
}

void Adafruit_HX8340B::writeData(uint8_t *data, uint count)
{
    waitUntilReady();

    pio_sm_put_blocking(_pio, _stateMachine, DATA);
    pio_sm_put_blocking(_pio, _stateMachine, count);
    while (count > 0) {
        pio_sm_put_blocking(_pio, _stateMachine, *data ++);
        count --;
    }
}

void Adafruit_HX8340B::setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t t0, t1;

    switch(_rotation) {
        case 1:
            t0 = WIDTH - 1 - y1;
            t1 = WIDTH - 1 - y0;
            y0 = x0;
            x0 = t0;
            y1 = x1;
            x1 = t1;
            break;
        case 2:
            t0 = x0;
            x0 = WIDTH  - 1 - x1;
            x1 = WIDTH  - 1 - t0;
            t0 = y0;
            y0 = HEIGHT - 1 - y1;
            y1 = HEIGHT - 1 - t0;
            break;
        case 3:
            t0 = HEIGHT - 1 - x1;
            t1 = HEIGHT - 1 - x0;
            x0 = y0;
            y0 = t0;
            x1 = y1;
            y1 = t1;
            break;
    }

    /*uint8_t rawData[] = { 
        COMMAND, 1, HX8340B_N_CASET,
        DATA,    4, 0, x0, 0, x1,   
        COMMAND, 1, HX8340B_N_PASET,
        DATA,    4, 0, y0, 0, y1,   
        COMMAND, 1, HX8340B_N_RAMWR 
    };

    waitUntilReady();
    dma_channel_transfer_from_buffer_now(_dmaChannel, &rawData[0], count_of(rawData));*/

    writeCommand(HX8340B_N_CASET); // Column addr set
    writeData(0); writeData(x0);   // X start
    writeData(0); writeData(x1);   // X end
    writeCommand(HX8340B_N_PASET); // Page addr set
    writeData(0); writeData(y0);   // Y start
    writeData(0); writeData(y1);   // Y end
    writeCommand(HX8340B_N_RAMWR);
}

void Adafruit_HX8340B::fillScreen(uint16_t c) 
{
    uint8_t x, y;

    setWindow(0, 0, _width-1, _height-1);

    for (y = _height; y > 0; y--)
    {
        for(x = _width; x > 0; x--) 
        {
            writeData16(c);
        }
    }
}

void Adafruit_HX8340B::pushColor(uint16_t color) 
{
    writeData16(color);
}

// the most basic function, set a single pixel
void Adafruit_HX8340B::drawPixel (int16_t x, int16_t y, uint16_t color) 
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
        return;

    setWindow(x, y, x, y);
    pushColor(color);
}

void Adafruit_HX8340B::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
    if((x < 0) || (x >= _width)     // Fully off left or right
               || (y >= _height))   // Fully off bottom
        return;

    int16_t y2 = y + h - 1;
    if(y2 < 0)                      // Fully off top
        return;

    if(y2 >= _height)               // Clip bottom
        h = _height - y;

    if(y < 0)                       // Clip top
    { 
        h += y; 
        y = 0; 
    }
  
    setWindow(x, y, x, y + h - 1);

    while (h--) {
        writeData16(color);
    }
}

void Adafruit_HX8340B::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    if((y < 0) || (y >= _height)    // Fully off top or bottom
               || (x >= _width))    // Fully off right
        return;

    int16_t x2 = x + w - 1;
    if(x2 < 0)                      // Fully off left
        return;

    if(x2 >= _width)                // Clip right
        w = _width - x;

    if(x < 0)                       // Clip left
    {
        w += x; 
        x = 0; 
    }
    setWindow(x, y, x + w - 1, y);

    while (w--) {
        writeData16(color);
    }
}

void Adafruit_HX8340B::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    // rudimentary clipping (drawChar w/big text requires this)
    if((x >= _width) || (y >= _height)) // Fully off right or bottom
        return; 
  
    int16_t x2, y2;
    if(((x2 = x + w - 1) < 0) || ((y2 = y + h - 1) < 0)) // Fully off left or top
        return; 
    
    if(x2 >= _width) // Clip right
        w = _width  - x; 
    
    if(x < 0) // Clip left
    {
        w += x; 
        x = 0; 
    }       
  
    if(y2 >= _height) // Clip bottom
        h = _height - y; 
    
    if(y < 0)         // Clip top
    {
        h += y; 
        y = 0; 
    }

    setWindow(x, y, x + w - 1, y + h - 1);
    int32_t i  = (int32_t)w * (int32_t)h;

    while(i--) {
        writeData16(color);
    }
}


uint16_t Adafruit_HX8340B::color565(uint8_t r, uint8_t g, uint8_t b) 
{
    //return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    //return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
    return (r & 0xF8) | ((b >> 3) << 8) | ((g & 0x1C) << 11) | ((g & 0xE0) >> 5);
}

void Adafruit_HX8340B::drawBitmap(const uint16_t *buffer)
{
    setWindow(0, 0, _width-1, _height-1);

    const uint len = WIDTH * HEIGHT * 2;
    waitUntilReady();
    pio_sm_put_blocking(_pio, _stateMachine, DATA);
    pio_sm_put_blocking(_pio, _stateMachine, len);

    dma_channel_transfer_from_buffer_now(_dmaChannel, buffer, len);
}

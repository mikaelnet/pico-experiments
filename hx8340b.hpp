#ifndef ADAFRUIT_HX8340B
#define ADAFRUIT_HX8340B

#include "gfx.hpp"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#define HX8340B_LCDWIDTH                  176
#define HX8340B_LCDHEIGHT                 220

// HX8340-B(N) Commands (used by BTL221722-276L)
#define HX8340B_N_NOP                     (0x00)
#define HX8340B_N_SWRESET                 (0x01)
#define HX8340B_N_RDDIDIF                 (0x04)
#define HX8340B_N_RDDST                   (0x09)
#define HX8340B_N_RDDPM                   (0x0A)
#define HX8340B_N_RDDMADCTL               (0x0B)
#define HX8340B_N_RDDCOLMOD               (0x0C)
#define HX8340B_N_RDDIM                   (0x0D)
#define HX8340B_N_RDDSM                   (0x0E)
#define HX8340B_N_RDDSDR                  (0x0F)
#define HX8340B_N_SLPIN                   (0x10)
#define HX8340B_N_SPLOUT                  (0x11)
#define HX8340B_N_PTLON                   (0x12)
#define HX8340B_N_NORON                   (0x13)
#define HX8340B_N_INVOFF                  (0x20)
#define HX8340B_N_INVON                   (0x21)
#define HX8340B_N_GAMSET                  (0x26)
#define HX8340B_N_DISPOFF                 (0x28)
#define HX8340B_N_DISPON                  (0x29)
#define HX8340B_N_CASET                   (0x2A)
#define HX8340B_N_PASET                   (0x2B)
#define HX8340B_N_RAMWR                   (0x2C)
#define HX8340B_N_RAMRD                   (0x2E)
#define HX8340B_N_RGBSET                  (0x2D)
#define HX8340B_N_PLTAR                   (0x30)
#define HX8340B_N_VSCRDEF                 (0x33)
#define HX8340B_N_TEOFF                   (0x34)
#define HX8340B_N_TEON                    (0x35)
#define HX8340B_N_MADCTL                  (0x36)
#define HX8340B_N_VSCRSADD                (0x37)
#define HX8340B_N_IDMOFF                  (0x38)
#define HX8340B_N_IDMON                   (0x39)
#define HX8340B_N_COLMOD                  (0x3A)
#define HX8340B_N_RDID1                   (0xDA)
#define HX8340B_N_RDID2                   (0xDB)
#define HX8340B_N_RDID3                   (0xDC)
#define HX8340B_N_SETOSC                  (0xB0)
#define HX8340B_N_SETPWCTR1               (0xB1)
#define HX8340B_N_SETPWCTR2               (0xB2)
#define HX8340B_N_SETPWCTR3               (0xB3)
#define HX8340B_N_SETPWCTR4               (0xB4)
#define HX8340B_N_SETPWCTR5               (0xB5)
#define HX8340B_N_SETDISCTRL              (0xB6)
#define HX8340B_N_SETFRMCTRL              (0xB7)
#define HX8340B_N_SETDISCYCCTRL           (0xB8)
#define HX8340B_N_SETINVCTRL              (0xB9)
#define HX8340B_N_RGBBPCTR                (0xBA)
#define HX8340B_N_SETRGBIF                (0xBB)
#define HX8340B_N_SETDODC                 (0xBC)
#define HX8340B_N_SETINTMODE              (0xBD)
#define HX8340B_N_SETPANEL                (0xBE)
#define HX8340B_N_SETOTP                  (0xC7)
#define HX8340B_N_SETONOFF                (0xC0)
#define HX8340B_N_SETEXTCMD               (0xC1)
#define HX8340B_N_SETGAMMAP               (0xC2)
#define HX8340B_N_SETGAMMAN               (0xC3)


class Adafruit_HX8340B : public Adafruit_GFX 
{
    public:
        //Adafruit_HX8340B(int8_t SID, int8_t SCLK, int8_t RST, int8_t CS); // Bit-banging
        //Adafruit_HX8340B(spi_inst_t *spi_port, int8_t mosi_pin, int8_t clk_pin, int8_t RST, int8_t CS);
        Adafruit_HX8340B(PIO pio, int8_t mosi_pin, int8_t clk_pin, int8_t RST);

        void begin();
        void end();
        void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
        void fillScreen(uint16_t c);
        void pushColor(uint16_t c);
        void drawPixel(int16_t x, int16_t y, uint16_t color);

        void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        
        void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c);

        uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

        void rawWrite(uint16_t color) { writeData16(color); };
        void drawBitmap(const uint16_t *buffer);

        bool isReady() { return !dma_channel_is_busy(_dmaChannel); };
        void waitUntilReady() { dma_channel_wait_for_finish_blocking(_dmaChannel); }

    private:
        void reset();


        void writeCommand(uint8_t c);
        void writeData(uint8_t c);
        void writeData16(uint16_t c);
        void writeData(uint8_t *data, uint count);

        PIO _pio;
        uint _stateMachine;
        int _dmaChannel;
        //spi_inst_t *_spi_port;
        uint32_t _mosi_pin;
        uint32_t _clk_pin;
        //uint8_t _cs_pin;
        uint8_t _rst_pin;
};



#endif // ADAFRUIT_HX8340B
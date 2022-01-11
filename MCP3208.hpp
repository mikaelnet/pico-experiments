#ifndef __MCP3208_HPP
#define __MCP3208_HPP

#include "hardware/spi.h"

/**
 * @brief Low level MCP3208 8-channel 12bit ADC SPI chip driver
 * 
 * Keep around 1MHz at 2.7V VCC and around 2MHz at 5V VCC
 */
class MCP3208
{
    private:
        spi_inst_t *_spi_port;
        uint8_t _cs_pin;

    public:
        MCP3208 (spi_inst_t *spi_port, uint8_t cs_pin);
        void begin();

        uint adcRead(uint channel);
};

#endif

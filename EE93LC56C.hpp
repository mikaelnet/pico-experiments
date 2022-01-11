#ifndef __EE93LC56C_HPP
#define __EE93LC56C_HPP

#include "hardware/spi.h"

/**
 * @brief Low level 93LC56C 2kbit SPI EEPROM
 */
class EE93LC56C
{
    private:
        spi_inst_t *_spi_port;
        uint8_t _cs_pin;

        void chipSelect(bool select);
        
    public:
        EE93LC56C (spi_inst_t *spi_port, uint8_t cs_pin);
        void begin();

        void erase(uint8_t address);
        void eraseAll();
};

#endif

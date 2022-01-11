#include "EE93LC56C.hpp"

#include "pico/stdlib.h"
#include "pico/types.h"

#include "hardware/spi.h"

#define ERASE   0x0E00
#define ERAL    0x0900
#define EWDS    0x0800
#define EWEN    0x0980
#define READ    0x0C00
#define WRITE   0x0B00
#define WRAL    0x0880

EE93LC56C::EE93LC56C (spi_inst_t *spi_port, uint8_t cs_pin)
{
    _spi_port = spi_port;
    _cs_pin = cs_pin;
}

void EE93LC56C::begin()
{
    gpio_init(_cs_pin);
    gpio_set_dir(_cs_pin, GPIO_OUT);
    chipSelect(false);
}

void EE93LC56C::chipSelect(bool select)
{
    gpio_put(_cs_pin, select);       // Note: This chip has CS active high
}

void EE93LC56C::erase(uint8_t address)
{

}

void EE93LC56C::eraseAll()
{

}

void EE93LC56C::read(uint8_t address, uint8_t *buf, uint size)
{
    uint8_t tx[2];
    tx[0] = (READ & 0xFF00) >> 8;
    tx[1] = address;

    chipSelect(true);
    spi_write_blocking(_spi_port, tx, 2);
    spi_read_blocking(_spi_port, 0x00, buf, size);
    chipSelect(false);
}


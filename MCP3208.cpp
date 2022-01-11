#include "MCP3208.hpp"

#include "pico/stdlib.h"
#include "pico/types.h"

#include "hardware/spi.h"


MCP3208::MCP3208 (spi_inst_t *spi_port, uint8_t cs_pin)
{
    _spi_port = spi_port;
    _cs_pin = cs_pin;
}

void MCP3208::begin()
{
    gpio_init(_cs_pin);
    gpio_set_dir(_cs_pin, GPIO_OUT);
    gpio_put(_cs_pin, 1);
}

uint MCP3208::adcRead(uint channel)
{
    channel = (0x10 | (channel & 0x07)) << 14;
    uint8_t tx[3], rx[3];
    tx[0] = (channel & 0x00FF0000) >> 16; 
    tx[1] = (channel & 0x0000FF00) >> 8; 
    tx[2] = (channel & 0x000000FF);

    gpio_put(_cs_pin, 0);
    spi_write_read_blocking(_spi_port, tx, rx, 3);
    gpio_put(_cs_pin, 1);
    return ((rx[1] && 0x0F) << 8) | rx[2];
}
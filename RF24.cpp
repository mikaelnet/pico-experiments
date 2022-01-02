/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "nRF24L01.h"
#include "RF24_config.h"
#include "RF24.hpp"

#include <string.h>

#ifndef LOW
#define LOW 0
#endif

#ifndef HIGH
#define HIGH 1
#endif

#ifndef _BV
#define _BV(x) (1 << (x))
#endif

static const uint8_t child_pipe[] = {
    RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5
};

static const uint8_t child_pipe_enable[] = {
    ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5
};

/****************************************************************************/
RF24::RF24(spi_inst_t *spi_port, uint mosi_pin, uint miso_pin, uint clk_pin, uint ce_pin, uint cs_pin, uint spi_speed)
{
    _spi_port = spi_port;
    _mosi_pin = mosi_pin;
    _miso_pin = miso_pin;
    _clk_pin = clk_pin;
    _ce_pin = ce_pin;
    _cs_pin = cs_pin;
    _spi_speed = spi_speed;
    _csDelay = 5;

    payload_size = 32;
    _is_p_variant = false;
    _is_p0_rx = false;
    addr_width = 5;
    dynamic_payloads_enabled = true;


    _init_obj();
}

/****************************************************************************/

void RF24::csn(bool mode)
{
    gpio_put(_cs_pin, mode);
    sleep_us(_csDelay);
}

/****************************************************************************/

void RF24::ce(bool level)
{
    gpio_put(_ce_pin, level);
}

/****************************************************************************/

inline void RF24::beginTransaction()
{
    csn(LOW);
}

/****************************************************************************/

inline void RF24::endTransaction()
{
    csn(HIGH);
}

/****************************************************************************/

void RF24::_init_obj()
{
    pipe0_reading_address[0] = 0;
}

/****************************************************************************/

void RF24::setChannel(uint8_t channel)
{
    const uint8_t max_channel = 125;
    write_register(RF_CH, rf24_min(channel, max_channel));
}

uint8_t RF24::getChannel()
{
    return read_register(RF_CH);
}

/****************************************************************************/

void RF24::setPayloadSize(uint8_t size)
{
    // payload size must be in range [1, 32]
    payload_size = static_cast<uint8_t>(rf24_max(1, rf24_min(32, size)));

    // write static payload size setting for all pipes
    for (uint8_t i = 0; i < 6; ++i) {
        write_register(static_cast<uint8_t>(RX_PW_P0 + i), payload_size);
    }
}

/****************************************************************************/

uint8_t RF24::getPayloadSize(void)
{
    return payload_size;
}

/****************************************************************************/

bool RF24::begin(void)
{
    spi_init(_spi_port, _spi_speed);
    gpio_set_function(_clk_pin, GPIO_FUNC_SPI);
    gpio_set_function(_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(_miso_pin, GPIO_FUNC_SPI);
    bi_decl(bi_3pins_with_func(_mosi_pin, _miso_pin, _clk_pin, GPIO_FUNC_SPI));
    spi_set_format(_spi_port, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    return _init_pins() && _init_radio();
}

/****************************************************************************/

void RF24::read_register(uint8_t reg, uint8_t* buf, uint8_t len)
{
    beginTransaction(); //configures the spi settings for RPi, locks mutex and setting csn low
    uint8_t *prx = spi_rxbuff;
    uint8_t *ptx = spi_txbuff;
    uint8_t size = len + 1;

    *ptx++ = (R_REGISTER | reg);
    while (len--) { 
        *ptx++ = RF24_NOP;  // Dummy operation, just for reading
    }

    spi_write_read_blocking(_spi_port, spi_txbuff, spi_rxbuff, size);
    status = *prx++; // status is 1st byte of receive buffer

    // decrement before to skip status byte
    while (--size) { 
        *buf++ = *prx++; 
    }

    endTransaction(); // unlocks mutex and setting csn high
}

/****************************************************************************/

uint8_t RF24::read_register(uint8_t reg)
{
    uint8_t result;

    beginTransaction();

    uint8_t *prx = spi_rxbuff;
    uint8_t *ptx = spi_txbuff;
    *ptx++ = (R_REGISTER | reg);
    *ptx++ = RF24_NOP ; // Dummy operation, just for reading

    spi_write_read_blocking(_spi_port, spi_txbuff, spi_rxbuff, 2);

    status = *prx;     // status is 1st byte of receive buffer
    result = *++prx;   // result is 2nd byte of receive buffer

    endTransaction();

    return result;
}

/****************************************************************************/

void RF24::write_register(uint8_t reg, const uint8_t* buf, uint8_t len)
{
    beginTransaction();
    uint8_t *prx = spi_rxbuff;
    uint8_t *ptx = spi_txbuff;
    uint8_t size = len + 1; // Add register value to transmit buffer

    *ptx++ = (W_REGISTER | (REGISTER_MASK & reg));
    while (len--) { 
        *ptx++ = *buf++; 
    }

    spi_write_read_blocking(_spi_port, spi_txbuff, spi_rxbuff, size);

    status = *prx; // status is 1st byte of receive buffer
    endTransaction();
}

/****************************************************************************/

void RF24::write_register(uint8_t reg, uint8_t value, bool is_cmd_only)
{
    beginTransaction();
    uint8_t *prx = spi_rxbuff;
    uint8_t *ptx = spi_txbuff;
    *ptx++ = (W_REGISTER | reg);
    *ptx = value;

    spi_write_read_blocking(_spi_port, spi_txbuff, spi_rxbuff, is_cmd_only ? 1 : 2);

    status = *prx++; // status is 1st byte of receive buffer
    endTransaction();
}

/****************************************************************************/

void RF24::write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
    const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

    uint8_t blank_len = !data_len ? 1 : 0;
    if (!dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, payload_size);
        blank_len = payload_size - data_len;
    }
    else {
        data_len = rf24_min(data_len, 32);
    }

    beginTransaction();
    uint8_t *prx = spi_rxbuff;
    uint8_t *ptx = spi_txbuff;
    uint8_t size;
    size = data_len + blank_len + 1; // Add register value to transmit buffer

    *ptx++ =  writeType;
    while (data_len--) { 
        *ptx++ = *current++; 
    }
    while (blank_len--) { 
        *ptx++ = 0; 
    }

    spi_write_read_blocking(_spi_port, spi_txbuff, spi_rxbuff, size);

    status = *prx; // status is 1st byte of receive buffer
    endTransaction();
}

/****************************************************************************/

void RF24::read_payload(void* buf, uint8_t data_len)
{
    uint8_t* current = reinterpret_cast<uint8_t*>(buf);

    uint8_t blank_len = 0;
    if (!dynamic_payloads_enabled) {
        data_len = rf24_min(data_len, payload_size);
        blank_len = payload_size - data_len;
    }
    else {
        data_len = rf24_min(data_len, 32);
    }

    beginTransaction();
    uint8_t * prx = spi_rxbuff;
    uint8_t * ptx = spi_txbuff;
    uint8_t size;
    size = data_len + blank_len + 1; // Add register value to transmit buffer

    *ptx++ =  R_RX_PAYLOAD;
    while(--size) { 
        *ptx++ = RF24_NOP; 
    }

    size = data_len + blank_len + 1; // Size has been lost during while, re affect

    spi_write_read_blocking(_spi_port, spi_txbuff, spi_rxbuff, size);
    status = *prx++; // 1st byte is status

    if (data_len > 0) {
        // Decrement before to skip 1st status byte
        while (--data_len) { 
            *current++ = *prx++; 
        }

        *current = *prx;
    }
    endTransaction();
}


/****************************************************************************/

uint8_t RF24::flush_rx(void)
{
    write_register(FLUSH_RX, RF24_NOP, true);
    return status;
}

/****************************************************************************/

uint8_t RF24::flush_tx(void)
{
    write_register(FLUSH_TX, RF24_NOP, true);
    return status;
}

/****************************************************************************/

uint8_t RF24::get_status(void)
{
    write_register(RF24_NOP, RF24_NOP, true);
    return status;
}


/****************************************************************************/

uint8_t RF24::spiTransfer(uint8_t data)
{
    uint8_t result = 0;
    spi_write_read_blocking(_spi_port, &data, &result, 1);
    return result;
}

/****************************************************************************/

void RF24::setAddressWidth(uint8_t a_width)
{
    a_width = a_width - 2;
    if (a_width) {
        write_register(SETUP_AW, a_width % 4);
        addr_width = (a_width % 4) + 2;
    } else {
        write_register(SETUP_AW, 0);
        addr_width = 2;
    }
}

/****************************************************************************/
void RF24::openReadingPipe(uint8_t child, const uint8_t* address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(pipe0_reading_address, address, addr_width);
        _is_p0_rx = true;
    }
    if (child <= 5) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            write_register(child_pipe[child], address, addr_width);
        } else {
            write_register(child_pipe[child], address, 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        uint8_t rxAddr = read_register(EN_RXADDR) | _BV(child_pipe_enable[child]);
        write_register(EN_RXADDR, rxAddr);

    }
}

/****************************************************************************/

void RF24::closeReadingPipe(uint8_t pipe)
{
    uint8_t rxAddr = read_register(EN_RXADDR) & ~_BV(child_pipe_enable[pipe]);
    write_register(EN_RXADDR, rxAddr);
    if (!pipe) {
        // keep track of pipe 0's RX state to avoid null vs 0 in addr cache
        _is_p0_rx = false;
    }
}

/****************************************************************************/

void RF24::toggle_features()
{
    beginTransaction();
    status = spiTransfer(ACTIVATE);
    spiTransfer(0x73);
    endTransaction();
}

/****************************************************************************/

bool RF24::_init_pins()
{
    if (!isValid()) {
        // didn't specify the CSN & CE pins to c'tor nor begin()
        return false;
    }

    gpio_init(_ce_pin);
    gpio_set_dir(_ce_pin, GPIO_OUT);
    ce(LOW);
    sleep_us(100);

    gpio_init(_cs_pin);
    gpio_set_dir(_cs_pin, GPIO_OUT);
    csn(HIGH);
    
    return true; // assuming pins are connected properly
}

/****************************************************************************/

bool RF24::_init_radio()
{
    // Must allow the radio time to settle else configuration bits will not necessarily stick.
    // This is actually only required following power up but some settling time also appears to
    // be required after resets too. For full coverage, we'll always assume the worst.
    // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
    // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
    // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
    sleep_ms(5);

    // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
    // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
    // sizes must never be used. See datasheet for a more complete explanation.
    setRetries(5, 15);

    // Then set the data rate to the slowest (and most reliable) speed supported by all
    // hardware. Since this value occupies the same register as the PA level value, set
    // the PA level to MAX
    setRadiation(RF24_PA_MAX, RF24_1MBPS); // LNA enabled by default

    // detect if is a plus variant & use old toggle features command accordingly
    uint8_t before_toggle = read_register(FEATURE);
    toggle_features();
    uint8_t after_toggle = read_register(FEATURE);
    _is_p_variant = before_toggle == after_toggle;
    if (after_toggle){
        if (_is_p_variant){
            // module did not experience power-on-reset (#401)
            toggle_features();
        }
        // allow use of multicast parameter and dynamic payloads by default
        write_register(FEATURE, 0);
    }
    ack_payloads_enabled = false;     // ack payloads disabled by default
    write_register(DYNPD, 0);         // disable dynamic payloads by default (for all pipes)
    dynamic_payloads_enabled = false;
    write_register(EN_AA, 0x3F);      // enable auto-ack on all pipes
    write_register(EN_RXADDR, 3);     // only open RX pipes 0 & 1
    setPayloadSize(32);               // set static payload size to 32 (max) bytes by default
    setAddressWidth(5);               // set default address length to (max) 5 bytes

    // Set up default configuration.  Callers can always change it later.
    // This channel should be universally safe and not bleed over into adjacent
    // spectrum.
    setChannel(76);

    // Reset current status
    // Notice reset and flush is the last thing we do
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));


    // Flush buffers
    flush_rx();
    flush_tx();

    // Clear CONFIG register:
    //      Reflect all IRQ events on IRQ pin
    //      Enable PTX
    //      Power Up
    //      16-bit CRC (CRC required by auto-ack)
    // Do not write CE high so radio will remain in standby I mode
    // PTX should use only 22uA of power
    write_register(NRF_CONFIG, (_BV(EN_CRC) | _BV(CRCO)) );
    config_reg = read_register(NRF_CONFIG);

    powerUp();

    // if config is not set correctly then there was a bad response from module
    return config_reg == (_BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP)) ? true : false;
}

/****************************************************************************/

bool RF24::isValid()
{
    return _ce_pin != 0xFFFF && _cs_pin != 0xFFFF;
}


/****************************************************************************/

void RF24::startListening(void)
{
    powerUp();
    config_reg |= _BV(PRIM_RX);
    write_register(NRF_CONFIG, config_reg);
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
    ce(HIGH);

    // Restore the pipe0 address, if exists
    if (_is_p0_rx) {
        write_register(RX_ADDR_P0, pipe0_reading_address, addr_width);
    } else {
        closeReadingPipe(0);
    }
}

/****************************************************************************/

void RF24::stopListening(void)
{
    ce(LOW);

    sleep_us(txDelay);
    if (ack_payloads_enabled){
        flush_tx();
    }

    config_reg = config_reg & ~_BV(PRIM_RX);
    write_register(NRF_CONFIG, config_reg);

    uint8_t rxAddr = read_register(EN_RXADDR) | _BV(child_pipe_enable[0]);
    write_register(EN_RXADDR, rxAddr); // Enable RX on pipe0
}

/****************************************************************************/

void RF24::powerDown(void)
{
    ce(LOW); // Guarantee CE is low on powerDown
    config_reg = static_cast<uint8_t>(config_reg & ~_BV(PWR_UP));
    write_register(NRF_CONFIG, config_reg);
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void RF24::powerUp(void)
{
    // if not powered up then power up and wait for the radio to initialize
    if (!(config_reg & _BV(PWR_UP))) {
        config_reg |= _BV(PWR_UP);
        write_register(NRF_CONFIG, config_reg);

        // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
        // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
        // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
        sleep_us(RF24_POWERUP_DELAY);
    }
}

/****************************************************************************/

void RF24::setRetries(uint8_t delay, uint8_t count)
{
    write_register(SETUP_RETR, static_cast<uint8_t>(rf24_min(15, delay) << ARD | rf24_min(15, count)));
}

/****************************************************************************/

uint8_t RF24::_data_rate_reg_value(rf24_datarate_e speed)
{
    txDelay = 280;
    if (speed == RF24_250KBPS) {
        txDelay = 505;
        // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
        // Making it '10'.
        return static_cast<uint8_t>(_BV(RF_DR_LOW));
    }
    else if (speed == RF24_2MBPS) {
        txDelay = 240;
        // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
        // Making it '01'
        return static_cast<uint8_t>(_BV(RF_DR_HIGH));
    }
    // HIGH and LOW '00' is 1Mbs - our default
    return static_cast<uint8_t>(0);
}

/****************************************************************************/

uint8_t RF24::_pa_level_reg_value(uint8_t level, bool lnaEnable)
{
    // If invalid level, go to max PA
    // Else set level as requested
    // + lnaEnable (1 or 0) to support the SI24R1 chip extra bit
    return static_cast<uint8_t>(((level > RF24_PA_MAX ? static_cast<uint8_t>(RF24_PA_MAX) : level) << 1) + lnaEnable);
}

/****************************************************************************/

void RF24::setRadiation(uint8_t level, rf24_datarate_e speed, bool lnaEnable)
{
    uint8_t setup = _data_rate_reg_value(speed);
    setup |= _pa_level_reg_value(level, lnaEnable);
    write_register(RF_SETUP, setup);
}


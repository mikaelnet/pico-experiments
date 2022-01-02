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

void RF24::openReadingPipe(uint8_t child, uint64_t address)
{
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) {
        memcpy(pipe0_reading_address, &address, addr_width);
        _is_p0_rx = true;
    }

    if (child <= 5) {
        // For pipes 2-5, only write the LSB
        if (child < 2) {
            write_register(child_pipe[child], reinterpret_cast<const uint8_t*>(&address), addr_width);
        } else {
            write_register(child_pipe[child], reinterpret_cast<const uint8_t*>(&address), 1);
        }

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR, static_cast<uint8_t>(read_register(EN_RXADDR) | _BV(child_pipe_enable[child])));
    }
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

bool RF24::available(void)
{
    return available(NULL);
}

/****************************************************************************/

bool RF24::available(uint8_t* pipe_num)
{
    // get implied RX FIFO empty flag from status byte
    uint8_t pipe = (get_status() >> RX_P_NO) & 0x07;
    if (pipe > 5)
        return 0;

    // If the caller wants the pipe number, include that
    if (pipe_num)
        *pipe_num = pipe;

    return 1;
}

/****************************************************************************/

void RF24::read(void* buf, uint8_t len)
{

    // Fetch the payload
    read_payload(buf, len);

    //Clear the only applicable interrupt flags
    write_register(NRF_STATUS, _BV(RX_DR));

}

/****************************************************************************/

void RF24::whatHappened(bool& tx_ok, bool& tx_fail, bool& rx_ready)
{
    // Read the status & reset the status in one easy call
    // Or is that such a good idea?
    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Report to the user what happened
    tx_ok = status & _BV(TX_DS);
    tx_fail = status & _BV(MAX_RT);
    rx_ready = status & _BV(RX_DR);
}

/****************************************************************************/

void RF24::openWritingPipe(uint64_t value)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.

    write_register(RX_ADDR_P0, reinterpret_cast<uint8_t*>(&value), addr_width);
    write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&value), addr_width);
}

/****************************************************************************/
void RF24::openWritingPipe(const uint8_t* address)
{
    // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
    // expects it LSB first too, so we're good.
    write_register(RX_ADDR_P0, address, addr_width);
    write_register(TX_ADDR, address, addr_width);
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

void RF24::enableDynamicPayloads(void)
{
    // Enable dynamic payload throughout the system

    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

    dynamic_payloads_enabled = true;
}

/****************************************************************************/
void RF24::disableDynamicPayloads(void)
{
    // Disables dynamic payload throughout the system.  Also disables Ack Payloads

    //toggle_features();
    write_register(FEATURE, 0);

    // Disable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, 0);

    dynamic_payloads_enabled = false;
    ack_payloads_enabled = false;
}

/****************************************************************************/

void RF24::enableAckPayload(void)
{
    // enable ack payloads and dynamic payload features

    if (!ack_payloads_enabled) {
        write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));

        // Enable dynamic payload on pipes 0 & 1
        write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
        dynamic_payloads_enabled = true;
        ack_payloads_enabled = true;
    }
}

/****************************************************************************/

void RF24::disableAckPayload(void)
{
    // disable ack payloads (leave dynamic payload features as is)
    if (ack_payloads_enabled){
        write_register(FEATURE, static_cast<uint8_t>(read_register(FEATURE) | ~_BV(EN_ACK_PAY)));

        ack_payloads_enabled = false;
    }
}

/****************************************************************************/

void RF24::enableDynamicAck(void)
{
    //
    // enable dynamic ack features
    //
    //toggle_features();
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DYN_ACK));
}

/****************************************************************************/

bool RF24::writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
    if (ack_payloads_enabled) {
        const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

        write_payload(current, len, W_ACK_PAYLOAD | (pipe & 0x07));
        return !(status & _BV(TX_FULL));
    }
    
    return 0;
}

/****************************************************************************/

bool RF24::isAckPayloadAvailable(void)
{
    return available(NULL);
}

/****************************************************************************/

bool RF24::isPVariant(void)
{
    return _is_p_variant;
}

/****************************************************************************/

void RF24::setAutoAck(bool enable)
{
    if (enable){
        write_register(EN_AA, 0x3F);
    }else{
        write_register(EN_AA, 0);
        // accomodate ACK payloads feature
        if (ack_payloads_enabled){
            disableAckPayload();
        }
    }
}

/****************************************************************************/

void RF24::setAutoAck(uint8_t pipe, bool enable)
{
    if (pipe < 6) {
        uint8_t en_aa = read_register(EN_AA);
        if (enable) {
            en_aa |= static_cast<uint8_t>(_BV(pipe));
        }else{
            en_aa = static_cast<uint8_t>(en_aa & ~_BV(pipe));
            if (ack_payloads_enabled && !pipe){
                disableAckPayload();
            }
        }
        write_register(EN_AA, en_aa);
    }
}

/****************************************************************************/

bool RF24::testCarrier(void)
{
    return (read_register(CD) & 1);
}

/****************************************************************************/

bool RF24::testRPD(void)
{
    return (read_register(RPD) & 1);
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

void RF24::errNotify()
{
    #if defined (SERIAL_DEBUG) || defined (RF24_LINUX)
    printf("RF24 HARDWARE FAIL: Radio not responding, verify pin connections, wiring, etc.\r\n");
    #endif
    failureDetected = 1;
}

/******************************************************************/

//Similar to the previous write, clears the interrupt flags
bool RF24::write(const void* buf, uint8_t len, const bool multicast)
{
    //Start Writing
    startFastWrite(buf, len, multicast);

    //Wait until complete or failed
    absolute_time_t timeout = make_timeout_time_us(95);

    while (!(get_status() & (_BV(TX_DS) | _BV(MAX_RT)))) {
        if (time_reached(timeout)) {
            errNotify();
            return 0;
        }
    }

    ce(LOW);

    write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    //Max retries exceeded
    if (status & _BV(MAX_RT)) {
        flush_tx(); // Only going to be 1 packet in the FIFO at a time using this method, so just flush
        return 0;
    }
    //TX OK 1 or 0
    return 1;
}

bool RF24::write(const void* buf, uint8_t len)
{
    return write(buf, len, 0);
}


/****************************************************************************/

bool RF24::writeFast(const void* buf, uint8_t len, const bool multicast)
{
    //Block until the FIFO is NOT full.
    //Keep track of the MAX retries and set auto-retry if seeing failures
    //Return 0 so the user can control the retrys and set a timer or failure counter if required
    //The radio will auto-clear everything in the FIFO as long as CE remains high
    absolute_time_t timeout = make_timeout_time_us(95);

    //Blocking only if FIFO is full. This will loop and block until TX is successful or fail
    while ((get_status() & (_BV(TX_FULL)))) {
        if (status & _BV(MAX_RT)) {
            return 0;                                        //Return 0. The previous payload has not been retransmitted
            // From the user perspective, if you get a 0, just keep trying to send the same payload
        }

        if (time_reached(timeout)) {
            errNotify();
            return 0;
        }
    }
    startFastWrite(buf, len, multicast);                     // Start Writing

    return 1;
}

bool RF24::writeFast(const void* buf, uint8_t len)
{
    return writeFast(buf, len, 0);
}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void RF24::startFastWrite(const void* buf, uint8_t len, const bool multicast, bool startTx)
{ //TMRh20

    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    if (startTx) {
        ce(HIGH);
    }
}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
bool RF24::startWrite(const void* buf, uint8_t len, const bool multicast)
{

    // Send the payload
    write_payload(buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);
    ce(HIGH);
    sleep_us(10);
    ce(LOW);
    return !(status & _BV(TX_FULL));
}


/****************************************************************************/

void RF24::setPALevel(uint8_t level, bool lnaEnable)
{
    uint8_t setup = read_register(RF_SETUP) & 0xF8;
    setup |= _pa_level_reg_value(level, lnaEnable);
    write_register(RF_SETUP, setup);
}

/****************************************************************************/

uint8_t RF24::getPALevel()
{

    return (read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1;
}

/****************************************************************************/

bool RF24::setDataRate(rf24_datarate_e speed)
{
    bool result = false;
    uint8_t setup = read_register(RF_SETUP);

    // HIGH and LOW '00' is 1Mbs - our default
    setup = setup & ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));
    setup |= _data_rate_reg_value(speed);

    write_register(RF_SETUP, setup);

    // Verify our result
    if (read_register(RF_SETUP) == setup) {
        result = true;
    }
    return result;
}

/****************************************************************************/

rf24_datarate_e RF24::getDataRate()
{
    rf24_datarate_e result;
    uint8_t dr = read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

    // switch uses RAM (evil!)
    // Order matters in our case below
    if (dr == _BV(RF_DR_LOW)) {
        // '10' = 250KBPS
        result = RF24_250KBPS;
    } else if (dr == _BV(RF_DR_HIGH)) {
        // '01' = 2MBPS
        result = RF24_2MBPS;
    } else {
        // '00' = 1MBPS
        result = RF24_1MBPS;
    }
    return result;
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


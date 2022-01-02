#include "pico/stdlib.h"
#include "pico/types.h"
#include "pico/printf.h"

#include <stdlib.h>
#include <string.h>

#include "RF24.hpp"
#include "packets.h"

#define RF24_MOSI   20
#define RF24_MISO   16
#define RF24_CLK    19
#define RF24_CE     16 // TODO: Set this pin
#define RF24_CS     17 // TODO: Set this pin

RF24 radio(spi1, RF24_MOSI, RF24_MISO, RF24_CLK, RF24_CE, RF24_CS, RF24_SPI_SPEED);
const uint64_t nRF_read_address = RECIEVER_ADDRESS;
const uint64_t nRF_write_address = TRANSMITTER_ADDRESS;

control_packet_t controlPacket;
metrics_packet_t metricsPacket;

void setup()
{
    stdio_init_all();
    setup_default_uart();
    printf("Hello from reciever!\n");

    if (!radio.begin())
    {
        printf("Radio hardware not responding\n");
    }

    metricsPacket.recieved = 0;

    radio.openWritingPipe(nRF_write_address);
    radio.openReadingPipe(1, nRF_read_address);

    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(0x34);   // TODO: Use a scanner to find a good value here
    radio.enableDynamicPayloads();
    radio.enableAckPayload();       // not used here
    radio.setRetries(0, 15);        // Smallest time between retries, max no. of retries
    radio.setAutoAck(true) ;

    radio.startListening();
}

void loop()
{
    if (radio.available())
    {
        radio.read(&controlPacket, sizeof(controlPacket));

        metricsPacket.recieved ++;

        radio.stopListening();
        radio.write( &metricsPacket, sizeof(metricsPacket) );
        radio.startListening();
    }
}

int main()
{
    setup();
    while (true)
    {
        loop();
    }
}

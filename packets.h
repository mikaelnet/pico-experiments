#include "pico/stdlib.h"

#ifndef __PACKETS_H
#define __PACKETS_H

typedef struct {
    uint16_t ch1;
    uint16_t ch2;
} control_packet_t;

typedef struct {
    uint16_t recieved;
} metrics_packet_t;

#define TRANSMITTER_ADDRESS     0x123456789A
#define RECIEVER_ADDRESS        0x123456789B

#endif


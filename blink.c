#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "blink.pio.h"

#define COMMAND 0x00
#define DATA 0x01

uint8_t buf[20];

int main() 
{
    PIO pio = pio0;

    uint locationOffset = pio_add_program (pio, &spi9_program);
    uint stateMachine = pio_claim_unused_sm(pio, true);
    spi9_program_init(pio, stateMachine, locationOffset, 31.25f, 2, 3);

    for (int i=0 ; i < count_of(buf) ; i ++)
        buf[i] = i;

    //gpio_init(5);
    //gpio_set_dir(5, GPIO_OUT);
    //gpio_put(5, 1);

    int chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, pio_get_dreq(pio, stateMachine, true));
    dma_channel_configure(chan, &c, &pio->txf[stateMachine], NULL, 0, false);


    uint8_t i = 0;
    while (true) {
        //while (!pio_sm_is_tx_fifo_empty(pio, stateMachine))
        //    ;

        //pio_sm_restart(pio, stateMachine);
        //gpio_put(5, 0);
        pio_sm_put_blocking(pio, stateMachine, COMMAND);
        pio_sm_put_blocking(pio, stateMachine, 0x02);
        pio_sm_put_blocking(pio, stateMachine, 0x55);
        pio_sm_put_blocking(pio, stateMachine, 0xAA);

        pio_sm_put_blocking(pio, stateMachine, DATA);
        pio_sm_put_blocking(pio, stateMachine, count_of(buf));
        dma_channel_transfer_from_buffer_now(chan, buf, count_of(buf));

        //channel_config_set_read_increment(&c, true);
        //channel_config_set_write_increment(&c, false);


        
        //pio_sm_put_blocking(pio, stateMachine, 0x55AA55AA);
        //pio_sm_put_blocking(pio, stateMachine, 0x12345678);

        //pio_sm_put_blocking(pio, stateMachine, DATA);
        //pio_sm_put_blocking(pio, stateMachine, 2);
        //pio_sm_put_blocking(pio, stateMachine, 0x55555555);
        //pio_sm_put_blocking(pio, stateMachine, 0xAAAAAAAA);

        //pio_sm_put_blocking(pio, stateMachine, command(1)); // 1 byte command
        //pio_sm_put_blocking(pio, stateMachine, 0x12345678);   // command
        //pio_sm_put_blocking(pio, stateMachine, (1 << 31) | 1);
        //gpio_put(5, 1);
        
        // sleep_us(10);
        dma_channel_wait_for_finish_blocking(chan);
        /*gpio_put(5, 0);
        pio_sm_put_blocking(pio, stateMachine, data(2)); // 2 byte data
        pio_sm_put_blocking(pio, stateMachine, 0xFEDCBA98);
        pio_sm_put_blocking(pio, stateMachine, 0x12345678);
        gpio_put(5, 1);
        sleep_us(1);*/
    }

    dma_channel_unclaim(chan);
}

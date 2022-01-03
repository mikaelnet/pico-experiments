#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "blink.pio.h"

int main() 
{
    PIO pio = pio0;

    uint locationOffset = pio_add_program (pio, &blink_program);

    uint stateMachine = pio_claim_unused_sm(pio, true);

    while (true) {
        blink_program_init(pio, stateMachine, locationOffset, PICO_DEFAULT_LED_PIN);
        for (int i=0 ; i < 3 ; i ++) {
            pio_sm_put_blocking(pio, stateMachine, 1);
            sleep_ms(500);
            pio_sm_put_blocking(pio, stateMachine, 0);
            sleep_ms(500);
        }

        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

        for (int i=0 ; i < 3 ; i ++) {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
            sleep_ms(100);
            gpio_put(PICO_DEFAULT_LED_PIN, false);
            sleep_ms(100);
        }
        sleep_ms(400);
    }
}

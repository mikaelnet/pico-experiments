#include "pico/stdlib.h"
#include "pico/types.h"
#include "pico/printf.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#include <stdlib.h>
#include <string.h>

#include "display.hpp"
#include "RF24.hpp"
#include "packets.h"


#define BTN_PIN     22

#define RF24_MOSI   11
#define RF24_MISO   12
#define RF24_CLK    10
#define RF24_CS     13
#define RF24_CE     14


RF24 radio(spi1, RF24_MOSI, RF24_MISO, RF24_CLK, RF24_CE, RF24_CS, RF24_SPI_SPEED);
const uint64_t nRF_read_address = TRANSMITTER_ADDRESS;
const uint64_t nRF_write_address = RECIEVER_ADDRESS;
control_packet_t controlPacket;
metrics_packet_t metricsPacket;

int button_pressed = 0;
absolute_time_t timeout = 0;

void push_button_callback(uint gpio, uint32_t events)
{
    if (gpio == BTN_PIN && events & GPIO_IRQ_EDGE_FALL) {
        if (time_reached(timeout)) {
            button_pressed ++;
            timeout = make_timeout_time_ms(50);
        }
    }
}

mutex_t radio_channels_mutex;

const uint8_t num_channels = 126;
uint channel_values[num_channels];

void core1_radio_scanner() 
{
    uint channel_pos = 0;

    while (true) 
    {
        radio.setChannel(channel_pos);
        radio.startListening();
        sleep_us(128);
        radio.stopListening();
        
        if (radio.testCarrier()) {
            mutex_enter_blocking(&radio_channels_mutex);
            channel_values[channel_pos] ++;
            mutex_exit(&radio_channels_mutex);
        }

        channel_pos ++;
        if (channel_pos >= num_channels)
            channel_pos = 0;
    }
}


absolute_time_t loopStart;

#define PWM_WRAP_POINT  500
void setup()
{
    stdio_init_all();
    setup_default_uart();
    mutex_init(&radio_channels_mutex);
    
    printf("Hello!\n");
    tft.begin();
    tft.fillScreen(BLACK);

    // Setup PWM for display backlight
    gpio_set_function(TFT_BL, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(TFT_BL);
    pwm_set_wrap(slice_num, PWM_WRAP_POINT);
    pwm_set_chan_level(slice_num, TFT_BL_CHAN, PWM_WRAP_POINT/2);
    pwm_set_enabled(slice_num, true);

    // Setup ADC for joystick
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(29);
    adc_set_temp_sensor_enabled(true);

    // Setup joystick push button interrupt
    gpio_init (BTN_PIN);
    gpio_pull_up(BTN_PIN);
    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &push_button_callback);
    timeout = make_timeout_time_ms(50);

    // nRF24l01 setup
    if (!radio.begin())
    {
        printf("Radio hardware not responding\n");
    }

    /*radio.openWritingPipe(nRF_write_address);
    radio.openReadingPipe(1, nRF_read_address);

    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(0x34);   // TODO: Use a scanner to find a good value here
    radio.enableDynamicPayloads();
    radio.enableAckPayload();       // not used here
    radio.setRetries(0, 15);        // Smallest time between retries, max no. of retries
    radio.setAutoAck(true);    

    radio.startListening();*/

    // Scanner code
    radio.setAutoAck(false);
    radio.startListening();
    radio.stopListening();

    metricsPacket.recieved = 0;

    memset (channel_values, 0, sizeof(channel_values));
    loopStart = get_absolute_time();
}

#define CROSS_HAIR_SIZE 4
uint16_t last_X, last_Y;

inline int map(int x, int in_min, int in_max, int out_min, int out_max) {  
    return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
}

uint drawTime_us = 0;
void loop()
{
    absolute_time_t computeStart = get_absolute_time();

    const uint adc_max = (1 << 12) - 1;
    adc_select_input(0);    // gpio 26, pin 31
    uint adc_x_raw = adc_read();
    adc_select_input(1);    // gpio 27, pin 32
    uint adc_y_raw = adc_read();
    
    adc_select_input(3);    // internal vbus
    float vbusVoltage = adc_read() * 3 * 3.3f / adc_max;
    
    adc_select_input(4);
    float tempVoltage = adc_read() * 3.3f / adc_max; 
    float temp = 27 - (tempVoltage - 0.706)/0.001721;

    printf("X: %d, Y %d, btn %d, temp: %f, vbus: %f V\n", adc_x_raw, adc_y_raw, button_pressed, temp, vbusVoltage);
    //printf("Channel: %d\n", radio.getChannel());
    //printf("PA level: %d\n", radio.getPALevel());
    //printf("Data rate: %d\n", radio.getDataRate());

    controlPacket.ch1 = adc_x_raw;
    controlPacket.ch2 = adc_y_raw;

    //radio.stopListening();
    //radio.write(&controlPacket, sizeof(controlPacket));
    //radio.startListening();

    uint bar_graphs[num_channels];
    mutex_enter_blocking(&radio_channels_mutex);
    memcpy(bar_graphs, channel_values, sizeof(bar_graphs));
    mutex_exit(&radio_channels_mutex);
    uint max = 0;
    for (int i=0; i < num_channels ; i ++) {
        if (bar_graphs[i] > max)
            max = bar_graphs[i];
    }
    if (max < 20)
        max = 20;

    uint left = (canvas->width() - num_channels)/2;
    uint bottom = canvas->height() - 5;
    
    uint16_t lineColor = tft.color565(0x80, 0x80, 0x80);
    canvas->drawFastHLine (left, bottom, num_channels, lineColor);
    canvas->drawFastVLine (left, bottom, 4, lineColor);
    canvas->drawFastVLine (left + num_channels/4, bottom, 2, lineColor);
    canvas->drawFastVLine (left + num_channels/2, bottom, 4, lineColor);
    canvas->drawFastVLine (left + num_channels*3/4, bottom, 2, lineColor);
    canvas->drawFastVLine (left + num_channels, bottom, 4, lineColor);

    const int graphHeight = 120;
    for (int i=0 ; i < num_channels ; i ++) {
        int height = map (bar_graphs[i], 0, max, 0, graphHeight);
        canvas->drawFastVLine(left + i, bottom - graphHeight, graphHeight - height, BLACK);
        canvas->drawFastVLine(left + i, bottom - height, height, WHITE);
    }


    uint16_t x = adc_x_raw * canvas->width() / adc_max;
    uint16_t y = adc_y_raw * canvas->height() / adc_max;

    // Fade screen according to joystick
    uint setPoint = adc_y_raw * PWM_WRAP_POINT / adc_max;
    uint slice_num = pwm_gpio_to_slice_num(TFT_BL);
    pwm_set_chan_level(slice_num, TFT_BL_CHAN, setPoint);


    canvas->drawFastVLine(last_X, last_Y - CROSS_HAIR_SIZE, CROSS_HAIR_SIZE * 2 + 1, BLACK);
    canvas->drawFastHLine(last_X - CROSS_HAIR_SIZE, last_Y, CROSS_HAIR_SIZE * 2 + 1, BLACK);
    canvas->drawFastVLine(x, y - CROSS_HAIR_SIZE, CROSS_HAIR_SIZE * 2 + 1, 0b0001000011111100);
    canvas->drawFastHLine(x - CROSS_HAIR_SIZE, y, CROSS_HAIR_SIZE * 2 + 1, 0b0001000011111100);
    last_X = x;
    last_Y = y;

    char buf[40];
    snprintf (buf, sizeof(buf), "X: %4d\nY: %4d\nBtn: %4d", adc_x_raw, adc_y_raw, button_pressed);
    canvas->setTextSize(1);
    canvas->setCursor(0,0);
    canvas->setTextColor(WHITE);
    drawtext(buf);

    /*absolute_time_t timeout = make_timeout_time_ms(20);
    while (!radio.available() && time_reached(timeout)) {
        tight_loop_contents();
    }

    if (radio.available()) {
        radio.read(&metricsPacket, sizeof(metricsPacket));
    }

    if (metricsPacket.recieved > 0) {
        snprintf (buf, sizeof(buf), "Reciever got %d packets", metricsPacket.recieved);
        display.setCursor(0, 4);
        display.setTextColor(GREEN);
        drawtext(buf);
    }*/

    uint frameTime_us = absolute_time_diff_us(loopStart, get_absolute_time());
    loopStart = get_absolute_time();

    uint computeTime_us = absolute_time_diff_us(computeStart, get_absolute_time());

    canvas->setTextSize(1);
    canvas->setTextColor(GREEN);

    snprintf(buf, sizeof(buf), "FPS %.2f", 1000000.0f/frameTime_us);
    canvas->setCursor(100, 0);
    drawtext(buf);
    snprintf(buf, sizeof(buf), "comp %dus", computeTime_us);
    canvas->setCursor(100, 12);
    drawtext(buf);
    snprintf(buf, sizeof(buf), "draw %dus", drawTime_us);
    canvas->setCursor(100, 24);
    drawtext(buf);

    if (tft.isReady())
    {
        absolute_time_t drawStart = get_absolute_time();
        tft.drawBitmap(canvas->getBuffer());
        canvas = (canvas == &canvas1) ? &canvas2 : &canvas1; // Swap canvas
        drawTime_us = absolute_time_diff_us(drawStart, get_absolute_time());
    }
}

#if false
void loop()
{
    absolute_time_t start = get_absolute_time();
    display.fillScreen(BLACK);
    //sleep_ms(500);

    display.drawPixel (display.width()/2, display.height()/2, GREEN);
    //sleep_ms(500);

     // line draw test
    testlines(YELLOW);
    //tft_display.drawBitmap(display.getBuffer());
    //sleep_ms(500);    
    
    // optimized lines
    testfastlines(RED, BLUE);
    //sleep_ms(500);    
    
    testdrawrects(GREEN);
    //sleep_ms(1000);

    testfillrects(YELLOW, MAGENTA);
    //sleep_ms(1000);

    display.fillScreen(BLACK);
    testfillcircles(10, BLUE);
    testdrawcircles(10, WHITE);
    //sleep_ms(1000);

    testroundrects();
    //tft_display.drawBitmap(display.getBuffer());
    //sleep_ms(500);
    
    testtriangles();
    //tft_display.drawBitmap(display.getBuffer());
    //sleep_ms(500);

    //lcdTestPattern();
    //sleep_ms(500);

    testdrawtext();
    //sleep_ms(1000);

    absolute_time_t end = get_absolute_time();
    int time = absolute_time_diff_us(start, end);

    char buf[40];
    snprintf (buf, sizeof(buf), "Time:\n%d us", time);

    display.fillScreen(BLACK);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(RED);
    drawtext("Hello world!\n");
    display.setTextColor(WHITE);
    drawtext(buf);

    //tft_display.drawBitmap(display.getBuffer());

    sleep_ms(5000);
}
#endif




int main()
{
    setup();
    multicore_launch_core1(core1_radio_scanner);
    while (true)
    {
        loop();
    }
}
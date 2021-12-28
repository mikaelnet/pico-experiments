# pico-experiments
Just playing around with a Raspberry PI Pico.

Connected an old discontinued 220x176px [Adafruit TFT display](https://www.adafruit.com/product/797). 
Contains as scaled-down port of the GFX library ported to rp2040.

**Objectives**
- [x] Get the toolchain running
- [x] Code in VS Code on PC. Compile on Raspberry PI 4. SWD flash programming.
- [x] Control the TFT using bit banging
- [x] Control the TFT using hardware spi port
- [ ] Investigate if a custom PIO 9-bit spi implementation would be better.
- [x] Attach a joystick and visualize its position
- [ ] Attach joystick button as command button
- [ ] Adjust TFT backlight with PWM
- [ ] Transmit data via a nRF24l01.

# Commands to remember
* `export PICO_SDK_PATH=/home/pi/pico/pico-sdk`
* from the build folder: `cmake .. && make -j4 && openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program remote.elf verify"`

# Hardware pins
| Pico pin | GPIO | Display PIN | Function   | Port |
| -------- | ---- | ----------- | ---------- | ---- |
| 4        | 2    | SCK         | SCK        | SPI0 |
| 5        | 3    | MOSI        | MOSI       | SPI0 |
| 6        | 4    | MISO        | MISO       | SPI0 |
| 7        | 5    | CS          | CS         | SPI0 |
| 12       | 9    | RST         | RESET      |      |
|          |      | BL          | Backlight  |      |
| 31       | 26   | VRx         | Joystick X | ADC0 |
| 32       | 27   | VRy         | Joystick Y | ADC1 |

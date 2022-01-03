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
- [x] Attach joystick button as command button
- [x] Adjust TFT backlight with PWM
- [x] Send debug data on usb tty
- [ ] Transmit data via a nRF24l01.
- [x] Radio scan for free air channels
- [x] Experiment with two cores - try mutexes
- [ ] Run on two cores, where one draws the TFT and the other does the com.
- [ ] Add a 8ch SPI ADC
- [ ] 

# Commands to remember
* `export PICO_SDK_PATH=/home/pi/pico/pico-sdk`
* from the build folder: `cmake .. && make -j4`
* `openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program transmitter.elf verify reset exit"`
* Terminal: `mimicom -b 115200 -o -D /dev/ttyACM0`

# Hardware pins
| Pico pin | GPIO | Display PIN | Function   | Port   |
| -------- | ---- | ----------- | ---------- | ------ |
| 4        | 2    | SCK         | SCK        | SPI0   |
| 5        | 3    | MOSI        | MOSI       | SPI0   |
| 6        | 4    | MISO        | MISO       | SPI0   |
| 7        | 5    | CS          | CS         | SPI0   |
| 12       | 9    | RST         | RESET      |        |
| 19       | 14   | BL          | Backlight  | PWM7 A |
| 31       | 26   | VRx         | Joystick X | ADC0   |
| 32       | 27   | VRy         | Joystick Y | ADC1   |

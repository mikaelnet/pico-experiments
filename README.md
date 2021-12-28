# pico-experiments
Just playing around with a Raspberry PI Pico.

Connected an old discontinued 220x176px [Adafruit TFT display](https://www.adafruit.com/product/797). 
Contains as scaled-down port of the GFX library ported to rp2040.

**Objectives**
- [x] Get the toolchain running
- [x] Code in VS Code on PC. Compile on Raspberry PI 4. SWD flash programming.
- [x] Control the TFT using bit banging
- [ ] Control the TFT using hardware spi port
- [ ] Investigate if a custom PIO 9-bit spi implementation would be better.
- [ ] Attach a joystick and visualize its position
- [ ] Transmit data via a nRF24l01.

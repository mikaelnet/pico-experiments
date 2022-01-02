cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(remote C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(transmitter
        tft-example.cpp
        gfx.cpp
        hx8340b.cpp
        gfxcanvas.cpp
        RF24.cpp
)

add_executable(reciever
        reciever.cpp
        RF24.cpp
)

# pull in common dependencies
target_link_libraries(transmitter 
        pico_stdlib
        pico_printf
        #pico_malloc #
        #pico_mem_ops #
        hardware_spi
        hardware_adc
        hardware_pwm
)

target_link_libraries(reciever 
        pico_stdlib 
        pico_printf 
        hardware_spi 
)

# enable usb output and disable uart on gpio 0/1
pico_enable_stdio_usb(transmitter 1)
pico_enable_stdio_uart(transmitter 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(transmitter)
pico_add_extra_outputs(reciever)


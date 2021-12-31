cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(remote C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(remote
        tft-example.cpp
        gfx.cpp
        hx8340b.cpp
        gfxcanvas.cpp
)

# pull in common dependencies
target_link_libraries(remote 
        pico_stdlib 
        pico_printf 
        pico_malloc 
        pico_mem_ops 
        hardware_spi 
        hardware_adc
        hardware_pwm
)

# create map/bin/hex file etc.
pico_add_extra_outputs(remote)

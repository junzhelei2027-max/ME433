cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW3_mcp23008 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW3_mcp23008
    HW3_mcp23008_button_led.c
)

target_link_libraries(HW3_mcp23008
    pico_stdlib
    hardware_i2c
)

pico_add_extra_outputs(HW3_mcp23008)




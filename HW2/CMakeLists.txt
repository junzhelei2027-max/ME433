cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW2 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW2
    HW2.c
)

target_link_libraries(HW2
    pico_stdlib
    hardware_pwm
)

pico_enable_stdio_usb(HW2 1)
pico_enable_stdio_uart(HW2 0)

pico_add_extra_outputs(HW2)

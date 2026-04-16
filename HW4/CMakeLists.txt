cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW4 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW4
    main.c
    ssd1306.c
)

target_link_libraries(HW4
    pico_stdlib
    hardware_i2c
    hardware_adc
)

pico_add_extra_outputs(HW4)
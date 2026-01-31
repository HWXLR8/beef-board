# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if (WIN32)
    set(USERHOME $ENV{USERPROFILE})
else ()
    set(USERHOME $ENV{HOME})
endif ()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif ()
# ====================================================================================
set(PICO_BOARD beef-board CACHE STRING "Board type")
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(beef-board C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(beef-board
        beef.cpp
        beef.h
        rgb_helper.cpp
        rgb_helper.h
        tusb_config.h
        usb_descriptors.c
)

pico_set_program_name(beef-board "beef-board")
pico_set_program_version(beef-board "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(beef-board 0)
pico_enable_stdio_usb(beef-board 0)

# Add the standard library to the build
target_link_libraries(beef-board
        pico_stdlib
)

# Add the standard include files to the build
target_include_directories(beef-board PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_precompile_headers(beef-board PRIVATE
        "include/Arduino.h"
)

file(GLOB_RECURSE FASTLED_SRCS
        "FastLED/src/*.cpp"
        "include/Arduino.cpp"
)
add_library(fastled ${FASTLED_SRCS})
target_include_directories(fastled PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/FastLED/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_precompile_headers(fastled PRIVATE
        "include/Arduino.h"
)
target_link_libraries(fastled
        hardware_adc
        hardware_dma
        hardware_pio
        hardware_pwm
        hardware_spi
        hardware_timer
        pico_bootrom
        pico_stdlib
        tinyusb_board
        tinyusb_device
)
target_include_directories(fastled PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(beef-board
        hardware_adc
        hardware_timer
        fastled
)

pico_add_extra_outputs(beef-board)

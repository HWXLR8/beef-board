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
set(PICO_SDK_FETCH_FROM_GIT 1)
set(PICO_SDK_FETCH_FROM_GIT_TAG "tinyusb-update")
include(pico_sdk_import.cmake)

project(beef-board C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(beef-board
        adc.cpp
        adc.h
        analog_button.cpp
        analog_button.h
        axis.cpp
        axis.h
        beef.cpp
        beef.h
        combo.cpp
        combo.h
        config.cpp
        config.h
        fastled.cpp
        fastled.h
        hid.cpp
        hid.h
        limiter.h
        patterns.cpp
        patterns.h
        rgb.cpp
        rgb.h
        ticker.cpp
        ticker.h
        timer.h
        tusb_config.h
        usb_descriptors.cpp
        usb_descriptors.h
        usb_handler.h
        ws2812.cpp
        ws2812.h

        devices/iidx/iidx_combo.cpp
        devices/iidx/iidx_combo.h
        devices/iidx/iidx_rgb.cpp
        devices/iidx/iidx_rgb.h
        devices/iidx/iidx_usb.cpp
        devices/iidx/iidx_usb.h

        devices/sdvx/sdvx_combo.cpp
        devices/sdvx/sdvx_combo.h
        devices/sdvx/sdvx_usb.cpp
        devices/sdvx/sdvx_usb.h
        devices/sdvx/sdvx_rgb.h
)

pico_set_program_name(beef-board "beef-board")
pico_set_program_version(beef-board "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(beef-board 0)
pico_enable_stdio_usb(beef-board 1)

# Add the standard library to the build
target_link_libraries(beef-board
        pico_stdlib
)
target_link_options(beef-board PRIVATE
        "--specs=nano.specs"
)

# Add the standard include files to the build
target_include_directories(beef-board PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
pico_generate_pio_header(beef-board
        ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio
        ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
)
target_link_libraries(beef-board
        hardware_adc
        hardware_dma
        hardware_flash
        hardware_pio
        hardware_timer
        pico_bootrom
        pico_flash
        tinyusb_board
        tinyusb_device
)

pico_add_extra_outputs(beef-board)

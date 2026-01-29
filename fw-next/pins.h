#pragma once

#include <cstdio>
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

struct button_pin_t
{
    uint8_t input_pin;
    uint8_t led_pin;
};

constexpr static uint8_t NUM_BUTTONS = 11;
constexpr static button_pin_t button_pins[NUM_BUTTONS] = {
    { 30, 31 }, // B1
    { 28, 29 }, // B2
    { 24, 25 }, // B3
    { 22, 23 }, // B4
    { 20, 21 }, // B5
    { 18, 19 }, // B6
    { 16, 17 }, // B7
    { 37, 38 }, // B8
    { 12, 13 }, // B9
    { 4, 5 },   // B10
    { 6, 7 }    // B11
};
constexpr static uint64_t button_gpio_mask = 1 << button_pins[0].led_pin | 1 << button_pins[1].led_pin |
    1 << button_pins[2].led_pin | 1 << button_pins[3].led_pin | 1 << button_pins[4].led_pin |
    1 << button_pins[5].led_pin | 1 << button_pins[6].led_pin | 1ULL << button_pins[7].led_pin |
    1 << button_pins[8].led_pin | 1 << button_pins[9].led_pin | 1 << button_pins[10].led_pin;

constexpr static uint8_t tt_pins[] = {
    2,  // E1D2
    3,  // E1D1
    35, // E2D2
    36  // E2D1
};

constexpr static uint8_t adc_gpio_pins[] = {
    40, // AE1
    41  // AE2
};

constexpr static uint8_t BAR_DATA_PIN = 42;
constexpr static uint8_t TT_DATA_PIN = 43;

inline bool __no_inline_not_in_flash_func(get_bootsel_button)()
{
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
#define CS_BIT SIO_GPIO_HI_IN_QSPI_CSN_BITS
    bool button_state = !(sio_hw->gpio_hi_in & CS_BIT);

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

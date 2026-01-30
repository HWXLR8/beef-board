#include <unordered_map>

#include "pins.h"
#include "rgb_helper.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;
uint16_t button_lights = 0;
const std::unordered_map<uint, uint> gpio_button_mapping = {
    { 30, 0 },
    { 28, 1 },
    { 24, 2 },
    { 22, 3 },
    { 20, 4 },
    { 18, 5 },
    { 16, 6 },
    { 37, 7 },
    { 12, 8 },
    { 4, 9 },
    { 6, 10 }
};

struct __attribute__((packed)) hid_lights
{
    uint16_t buttons;
    rgb_light tt_lights;
    rgb_light bar_lights;
};

hid_lights lights;
// CRGB leds[24];

void hw_init()
{
    for (const auto [input_pin, led_pin] : button_pins)
    {
        gpio_init(input_pin);
        gpio_pull_up(input_pin);
        gpio_set_inover(input_pin, GPIO_OVERRIDE_INVERT);

        gpio_init(led_pin);
        gpio_set_dir(led_pin, GPIO_OUT);
    }
    for (const auto pin : tt_pins)
    {
        gpio_init(pin);
        gpio_pull_up(pin);
        gpio_set_inover(pin, GPIO_OVERRIDE_INVERT);
    }

    // adc_init();
    // for (const auto adc_pin : adc_gpio_pins)
    // {
    //     adc_gpio_init(adc_pin);
    // }

    // gpio_init(TT_DATA_GPIO);
    // gpio_set_dir(TT_DATA_GPIO, GPIO_OUT);
    // FastLED.addLeds<NEOPIXEL, TT_DATA_GPIO>(leds, 24);

    // reboot to bootloader if B1 and B2 are held on startup
    if (gpio_get(button_pins[0].input_pin) && gpio_get(button_pins[1].input_pin))
        rom_reset_usb_boot(0, 0);
}

void process_buttons()
{
    for (auto i = 0; i < NUM_BUTTONS; i++)
    {
        const auto button_pin = button_pins[i];
        auto v = gpio_get(button_pin.input_pin);
        gpio_put(button_pin.led_pin, v);
    }
}

void process_tt()
{
    constexpr auto direction = 1;
    constexpr auto opposite_direction = -1;
    constexpr int8_t tt_transitions[4][4] = {
        { 0, direction, opposite_direction, 0 },
        { opposite_direction, 0, 0, direction },
        { direction, 0, 0, opposite_direction },
        { 0, opposite_direction, direction, 0 }
    };
    static uint8_t h = 0;

    static uint8_t prev = 0;
    const uint8_t a = gpio_get(tt_pins[0]);
    const uint8_t b = gpio_get(tt_pins[1]);
    const uint8_t curr = (a << 1) | b;

    const auto dir = tt_transitions[prev][curr];
    prev = curr;
}

[[noreturn]] int main()
{
    stdio_init_all();
    hw_init();

    while (true)
    {
        if (get_bootsel_button())
            rom_reset_usb_boot(0, 0);

        process_buttons();
        process_tt();
    }
}

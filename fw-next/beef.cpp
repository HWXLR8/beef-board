#include <unordered_map>

#include "pins.h"
#include "rgb_helper.h"
#include "tusb.h"
#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;
uint8_t tt_x = 0;
uint32_t hid_expiry_time = 0;

struct __attribute__((packed)) hid_lights
{
    uint16_t buttons;
    rgb_light tt_lights;
    rgb_light bar_lights;
};

hid_lights lights;
// CRGB leds[24];

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen)
{
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize)
{
    (void)itf;
    (void)report_id;
    (void)report_type;

    memcpy(&lights, buffer, bufsize);
    hid_expiry_time = board_millis() + 1000;
}

static void send_hid_report()
{
    // skip if hid is not ready yet
    if (!tud_hid_ready())
    {
        return;
    }

    struct __attribute__((packed)) joystick_report_data_t
    {
        uint8_t X;
        uint8_t Y; // Needed for LR2 compatibility
        uint16_t Button; // bit-field representing which buttons have been pressed
    };

    joystick_report_data_t report = {
        .X = tt_x,
        .Y = 127
    };

    // Infinitas only reads buttons 1-7, 9-12,
    // so shift bits 8 and up once
    const uint8_t upper = button_state >> 7;
    const uint8_t lower = button_state & 0x7F;
    report.Button = (upper << 8) | lower;

    tud_hid_report(0, &report, sizeof(report));
}

// Every millisecond, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task()
{
    // Poll every millisecond
    constexpr uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
    {
        return; // not enough time
    }
    start_ms += interval_ms;

    send_hid_report();
}

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

void usb_init()
{
    board_init();
    tusb_rhport_init_t dev_init = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
}

void process_buttons()
{
    button_state = 0;
    for (auto i = 0; i < NUM_BUTTONS; i++)
    {
        const auto button_pin = button_pins[i];
        auto v = gpio_get(button_pin.input_pin);
        button_state |= v << i;
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

    tt_x += dir;
}

void process_lights()
{
    const auto leds = board_millis() > hid_expiry_time ? button_state : lights.buttons;
    for (auto i = 0; i < NUM_BUTTONS; ++i)
    {
        gpio_put(button_pins[i].led_pin, leds & (1 << i));
    }
}

[[noreturn]] int main()
{
    stdio_init_all();
    hw_init();
    usb_init();

    while (true)
    {
        if (get_bootsel_button())
            rom_reset_usb_boot(0, 0);

        tud_task();

        process_buttons();
        process_tt();
        process_lights();

        hid_task();
    }
}

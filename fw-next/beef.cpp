#include "beef.h"

#include "analog_button.h"
#include "axis.h"
#include "combo.h"
#include "config.h"
#include "hid.h"
#include "pins.h"
#include "tusb.h"
#include "ws2812.h"
#include "bsp/board_api.h"
#include "devices/iidx/iidx_rgb.h"
#include "devices/iidx/iidx_usb.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "pico/stdio.h"

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;
bool reactive_leds = true;
hid_lights_t lights;

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

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize)
{
    (void)itf;
    (void)report_id;
    (void)report_type;

    if (report_type != HID_REPORT_TYPE_OUTPUT)
    {
        printf("tud_hid_set_report_cb: received report type %d\n", report_type);
        return;
    }

    memcpy(&lights, buffer, bufsize);
    hid_expiry_timer.arm(1000);
}

void tud_hid_report_failed_cb(uint8_t instance, hid_report_type_t report_type, uint8_t const* report,
                              uint16_t xferred_bytes)
{
    (void)report;

    printf("failed to send report type %d: instance: %d, xferred_bytes %d\n", report_type, instance, xferred_bytes);
}

void send_hid_report()
{
    struct __attribute__((packed)) joystick_report_data_t
    {
        uint8_t X = 0;
        uint8_t Y = 127; // Needed for LR2 compatibility
        uint16_t Buttons = 0; // bit-field representing which buttons have been pressed
    };
    static joystick_report_data_t report;

    // Infinitas only reads buttons 1-7, 9-12,
    // so shift bits 8 and up once
    const uint8_t upper = button_state >> 7;
    const uint8_t lower = button_state & 0x7F;
    report.X = tt_x.get();
    report.Buttons = (upper << 8) | lower;

    tud_hid_report(0, &report, sizeof(report));
}

void hid_task()
{
    // Poll every millisecond
    constexpr uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    const auto now = board_millis();
    if (now - start_ms < interval_ms || // not enough time
        !tud_hid_ready()) // skip if hid is not ready yet
    {
        return;
    }
    start_ms = now;

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

    // adc_init();
    // for (const auto adc_pin : adc_gpio_pins)
    // {
    //     adc_gpio_init(adc_pin);
    // }

    // reboot to bootloader if B1 and B2 are held on startup
    process_buttons(0);
    if (button_state == (BUTTON_1 | BUTTON_2))
        rom_reset_usb_boot(0, 0);
}

void controller_init()
{
    combo_init();
    IIDX::RgbManager::init();
    button_x = new AnalogButton(config.tt_deadzone, true);
}

void usb_init()
{
    stdio_init_all();
    board_init();
    tusb_rhport_init_t dev_init = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();
}

void process_buttons(int8_t tt1_report)
{
    button_state = 0;
    for (auto i = 0; i < NUM_BUTTONS; i++)
    {
        const auto button_pin = button_pins[i];
        auto v = gpio_get(button_pin.input_pin);
        button_state |= v << i;
    }

    switch (tt1_report)
    {
    case -1:
        button_state |= IIDX::BUTTON_TT_NEG;
        break;
    case 1:
        button_state |= IIDX::BUTTON_TT_POS;
        break;
    default:
        break;
    }
}

void process_lights(int8_t tt1_report)
{
    uint16_t led_state = lights.buttons;
    if (reactive_leds || !hid_expiry_timer.is_active())
        led_state = button_state;

    if (config.disable_leds ||
        // Temporarily black out button LEDs to notify a setting change
        combo_lights_timer.is_active())
    {
        led_state = 0;
    }

    for (auto i = 0; i < NUM_BUTTONS; ++i)
    {
        gpio_put(button_pins[i].led_pin, led_state & (1 << i));
    }

    if (ready_to_show())
    {
        IIDX::RgbManager::update(tt1_report, lights);
        ws2812_show();
    }
}

[[noreturn]] int main()
{
    hw_init();
    config_init();
    controller_init();
    ws2812_init();
    usb_init();

    while (true)
    {
#ifndef NDEBUG
        if (get_bootsel_button())
            rom_reset_usb_boot(0, 0);
#endif

        tud_task();

        tt_x.poll();
        const auto tt1_report = button_x->poll(tt_x.get());

        process_buttons(tt1_report);
        process_combos();
        process_lights(tt1_report);

        hid_task();

        /*static uint32_t last_show = 0;
        static uint32_t ticks = 0;
        ticks++;
        const auto now = board_millis();
        if (now - last_show >= 1000)
        {
            printf("ticks: %d\n", ticks);
            ticks = 0;
            last_show = now;
        }*/
    }
}

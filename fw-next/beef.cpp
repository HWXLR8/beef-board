#include "beef.h"

#include "analog_button.h"
#include "combo.h"
#include "config.h"
#include "hid.h"
#include "pins.h"
#include "tusb.h"
#include "ws2812.h"
#include "bsp/board_api.h"
#include "devices/iidx/iidx_usb.h"
#include "devices/sdvx/sdvx_usb.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "pico/stdio.h"

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;
bool reactive_leds = true;
usb_handler* usb;

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

    usb->hid_set_report(itf, report_id, report_type, buffer, bufsize);
    hid_expiry_timer.arm(1000);
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

    usb->send_hid_report();
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

    // reboot to bootloader if B1 and B2 are held on startup
    process_buttons();
    if (button_state == (BUTTON_1 | BUTTON_2))
        rom_reset_usb_boot(0, 0);
}

void controller_init()
{
    switch (button_state)
    {
    case BUTTON_1 | BUTTON_8:
        config.controller_type = ControllerType::IIDX;
        config.iidx_input_mode = InputMode::Joystick;
        config.save();
        break;
    /*case BUTTON_2 | BUTTON_8:
        set_controller_type(config, ControllerType::IIDX);
        set_input_mode(config, InputMode::Keyboard);
        break;*/
    case BUTTON_1 | BUTTON_9:
        config.controller_type = ControllerType::SDVX;
        config.iidx_input_mode = InputMode::Joystick;
        config.save();
        break;
    /*case BUTTON_2 | BUTTON_9:
        set_controller_type(config, ControllerType::SDVX);
        set_input_mode(config, InputMode::Keyboard);
        break;*/
    default:
        break;
    }

    switch (config.controller_type)
    {
    case ControllerType::IIDX:
        usb = new IIDX::usb_handler();
        break;
    case ControllerType::SDVX:
        usb = new SDVX::usb_handler();
        break;
    }
}

void usb_init()
{
    board_init();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    if (board_init_after_tusb)
    {
        board_init_after_tusb();
    }

    stdio_init_all();
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

void process_lights()
{
    uint16_t led_state = usb->get_button_light_state();
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
        usb->update_lighting();
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
        tud_task();

        process_buttons();
        process_combos();
        usb->update();
        process_lights();

        hid_task();
    }
}

#include "beef.h"

#include "analog_button.h"
#include "combo.h"
#include "config.h"
#include "hid.h"
#include "pins.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ws2812.h"
#include "bsp/board_api.h"
#include "devices/iidx/iidx_usb.h"
#include "devices/sdvx/sdvx_usb.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/stdio.h"

enum class command_t : uint8_t
{
    None,
    Bootloader,
    ResetConfig
};

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;
bool reactive_leds = true;
// Ignore buttons after bootup sequence
bool ignore_buttons = false;
auto current_command = command_t::None;
usb_handler* usb;

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;

    clear_all_lights();
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen)
{
    if (report_type != HID_REPORT_TYPE_FEATURE || ITF_HID_BASE + instance != ITF_NUM_CONFIG)
        return 0;

    switch (report_id)
    {
    case REPORT_ID_CONFIG:
        reqlen = sizeof(config) - sizeof(config.magic);
        memcpy(buffer, &config.version, reqlen);
        return reqlen;
    case REPORT_ID_FWVER:
        {
            constexpr uint32_t firmware_version = FW_VER;
            memcpy(buffer, &firmware_version, reqlen);
            return reqlen;
        }
    default:
        return 0;
    }
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize)
{
    switch (report_type)
    {
    case HID_REPORT_TYPE_OUTPUT:
        usb->hid_set_report(instance, report_id, report_type, buffer, bufsize);
        break;
    case HID_REPORT_TYPE_FEATURE:
        switch (report_id)
        {
        case REPORT_ID_CONFIG:
            printf("received feature config report: bufsize: %d\n", bufsize);
            memcpy(&config.version, buffer, bufsize);
            break;
        case REPORT_ID_COMMAND:
            memcpy(&current_command, buffer, bufsize);
            break;
        default:
            break;
        }
    default:
        break;
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    usb->hid_report_complete(instance, report, len);
}

void send_keyboard_report(const uint8_t* const key_codes, const uint8_t n)
{
    struct TU_ATTR_PACKED keyboard_report_data_t
    {
        uint8_t key_code[KEYBOARD_KEYS];
    };
    static keyboard_report_data_t report;

    uint8_t used_key_codes = 0;
    for (uint8_t i = 0; i < n; i++)
    {
        uint8_t key_code = 0;
        if (button_state & 1 << i)
        {
            key_code = key_codes[i];
        }
        report.key_code[used_key_codes++] = key_code;
    }

    tud_hid_report(REPORT_ID_KEYBOARD, &report, sizeof(report));
}

void send_mouse_report(const int8_t x, const int8_t y)
{
    struct TU_ATTR_PACKED mouse_report_data_t
    {
        int8_t X;
        int8_t Y;
    };
    static mouse_report_data_t report;

    report.X = x;
    report.Y = y;

    tud_hid_report(REPORT_ID_MOUSE, &report, sizeof(report));
}

void hid_task()
{
    // Poll every millisecond
    constexpr uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    const auto now = tusb_time_millis_api();
    if (now - start_ms < interval_ms || // not enough time
        !tud_hid_ready()) // skip if hid is not ready yet
    {
        return;
    }
    start_ms = now;

    usb->send_hid_report();
}

[[noreturn]] void reboot()
{
    tud_disconnect();
    watchdog_enable(250, false);
    while (true);
}

void reboot_to_bootloader()
{
    tud_disconnect();
    clear_all_lights();
    rom_reset_usb_boot(0, 0);
}

void handle_command()
{
    switch (current_command)
    {
    case command_t::None:
        return;
    case command_t::Bootloader:
        reboot_to_bootloader();
    case command_t::ResetConfig:
        config.version = 0;
        config.save();
        reboot();
    }
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

    ignore_buttons = true;
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
    case BUTTON_2 | BUTTON_8:
        config.controller_type = ControllerType::IIDX;
        config.iidx_input_mode = InputMode::Keyboard;
        config.save();
        break;
    case BUTTON_1 | BUTTON_9:
        config.controller_type = ControllerType::SDVX;
        config.sdvx_input_mode = InputMode::Joystick;
        config.save();
        break;
    case BUTTON_2 | BUTTON_9:
        config.controller_type = ControllerType::SDVX;
        config.sdvx_input_mode = InputMode::Keyboard;
        config.save();
        break;
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
    board_init_after_tusb();

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

    // Ignore button inputs after startup
    ignore_buttons = ignore_buttons && button_state;
    // If we are still ignoring button inputs, clear button_state
    // Otherwise, retain
    button_state *= !ignore_buttons;
}

void clear_all_lights()
{
    update_button_lighting(0);
    std::fill(bar_leds.begin(), bar_leds.end(), rgb_t{});
    std::fill(tt_leds.begin(), tt_leds.end(), rgb_t{});
    while (!ready_to_show());
    ws2812_show();
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

    update_button_lighting(led_state);

    if (ready_to_show())
    {
        usb->update_lighting();
        ws2812_show();
    }
}

void update_button_lighting(uint16_t led_state)
{
    for (auto i = 0; i < NUM_BUTTONS; ++i)
    {
        gpio_put(button_pins[i].led_pin, led_state & (1 << i));
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
        if (tud_suspended())
            continue;
        handle_command();

        process_buttons();
        process_combos();
        usb->update();
        process_lights();

        hid_task();
    }
}

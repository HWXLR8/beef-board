#include "iidx_usb.h"

#include "analog_button.h"
#include "axis.h"
#include "combo.h"
#include "config.h"
#include "iidx_defs.h"
#include "iidx_rgb.h"
#include "pins.h"
#include "usb_descriptors.h"
#include "ws2812.h"

namespace IIDX
{
    hid_lights_t lights{};
    Axis* axis_x;
    int8_t tt1_report = 0;

    usb_handler::usb_handler()
    {
        RgbManager::init();
        axis_x = new QeAxis(tt_pins[0]);
        button_x = new AnalogButton(config.tt_deadzone, true);
    }

    void usb_handler::send_hid_report()
    {
        switch (config.iidx_input_mode)
        {
        case InputMode::Joystick:
            {
                struct TU_ATTR_PACKED joystick_report_data_t
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
                report.X = axis_x->get();
                report.Buttons = (upper << 8) | lower;

                tud_hid_report(REPORT_ID_JOYSTICK, &report, sizeof(report));
                break;
            }
        case InputMode::Keyboard:
            send_keyboard_report(config.iidx_keys.key_codes, sizeof(config.iidx_keys.key_codes));
            break;
        }
    }

    void usb_handler::hid_set_report(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                                     uint8_t const* buffer, uint16_t bufsize)
    {
        switch (ITF_HID_BASE + instance)
        {
        case ITF_NUM_HID:
            {
                if (report_id != REPORT_ID_JOYSTICK)
                {
                    if (bufsize != sizeof(hid_lights_t) - 1)
                        return;
                    report_id = buffer[0];
                    if (report_id != REPORT_ID_JOYSTICK)
                        return;
                    // skip report id since tinyusb for some reason doesn't omit it for output reports
                    buffer++;
                    bufsize--;
                }
                assert(bufsize == sizeof(hid_lights_t));
                memcpy(&lights, buffer, bufsize);
                hid_expiry_timer.arm(1000);
                break;
            }
        case ITF_NUM_LIGHTS:
            assert(bufsize == sizeof(RgbManager::Bar::tape_leds));
            memcpy(&RgbManager::Bar::tape_leds, buffer, bufsize);
            lights_expiry_timer.arm(1000);
            break;
        default:
            break;
        }
    }

    void usb_handler::update()
    {
        axis_x->poll();
        tt1_report = button_x->poll(axis_x->get());

        switch (tt1_report)
        {
        case -1:
            button_state |= BUTTON_TT_NEG;
            break;
        case 1:
            button_state |= BUTTON_TT_POS;
            break;
        default:
            break;
        }
    }

    void usb_handler::update_lighting()
    {
        RgbManager::update(tt1_report, lights);
    }

    combo_t usb_handler::get_button_combo()
    {
        return IIDX::get_button_combo();
    }

    uint16_t usb_handler::get_button_light_state()
    {
        return lights.buttons;
    }

    void usb_handler::on_combo_reset()
    {
        IIDX::on_combo_reset();
    }

    void usb_handler::on_config_push()
    {
    }

    static constexpr tusb_desc_device_t desc_device = generate_device_descriptor(0x1CCF, 0x8048);

    tusb_desc_device_t const* usb_handler::get_descriptor_device()
    {
        return &desc_device;
    }

    //@formatter:off
    constexpr uint8_t desc_hid_report[] =
    {
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
        HID_USAGE(HID_USAGE_DESKTOP_JOYSTICK),
        HID_COLLECTION(HID_COLLECTION_APPLICATION),
            // Analog
            HID_REPORT_ID(REPORT_ID_JOYSTICK)
            HID_USAGE(HID_USAGE_DESKTOP_POINTER),
            HID_COLLECTION(HID_COLLECTION_LOGICAL),
                HID_USAGE(HID_USAGE_DESKTOP_X),
                HID_USAGE(HID_USAGE_DESKTOP_Y),
                HID_LOGICAL_MIN(0),
                HID_LOGICAL_MAX_N(255, 2),
                HID_PHYSICAL_MIN_N(-1, 2),
                HID_PHYSICAL_MAX(1),
                HID_REPORT_COUNT(0x02),
                HID_REPORT_SIZE(0x08),
                HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
            HID_COLLECTION_END,

            // Buttons
            // 11 physical (7 + 1 padding (Infinitas) + 4 extra) + digital TT (-/+)
            HID_BUTTONS(14),

            // Button lighting
            HID_BUTTON_LIGHT(1),
            HID_BUTTON_LIGHT(2),
            HID_BUTTON_LIGHT(3),
            HID_BUTTON_LIGHT(4),
            HID_BUTTON_LIGHT(5),
            HID_BUTTON_LIGHT(6),
            HID_BUTTON_LIGHT(7),
            HID_BUTTON_LIGHT(8),
            HID_BUTTON_LIGHT(9),
            HID_BUTTON_LIGHT(10),
            HID_BUTTON_LIGHT(11),
            HID_PADDING_OUTPUT(5),

            // TT WS2812
            HID_RGB(12),

            // Bar WS2812
            HID_RGB(15),
        HID_COLLECTION_END,

        HID_REPORT_DESC_KEYBOARD
    };

    constexpr uint8_t desc_lights_report[] =
    {
        HID_USAGE_PAGE_N(0xFFFB, 2),
        HID_USAGE(0x02),
        HID_LOGICAL_MIN(0x00),
        HID_LOGICAL_MAX_N(0xFF, 2),
        HID_COLLECTION(HID_COLLECTION_APPLICATION),
            // Tape LED
            HID_USAGE(1),
            HID_REPORT_SIZE(0x08 * 3),
            HID_REPORT_COUNT(LIGHT_BAR_LEDS),
            HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
        HID_COLLECTION_END
    };
    //@formatter:on

    const uint8_t* usb_handler::get_hid_descriptor_report(uint8_t instance)
    {
        if (ITF_HID_BASE + instance == ITF_NUM_LIGHTS) return desc_lights_report;
        return desc_hid_report;
    }

    void usb_handler::hid_report_complete(uint8_t instance, uint8_t const* report, uint16_t len)
    {
    }

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN)
    constexpr uint8_t desc_configuration[] =
    {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 500),

        // Interface number, string index, notification EP, notification EP size, EP Out & In address, EP Out & In size
        TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 0, EPNUM_CDC_CMD, 8, EPNUM_CDC, 0x80 | EPNUM_CDC, 64),

        // Interface number, string index, protocol, report descriptor len, EP Out & In address, size & polling interval
        TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID,
                                 0x80 | EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1),

        CONFIG_DESCRIPTOR,

        // Interface number, string index, protocol, report descriptor len, EP Out address, size & polling interval
        TUD_HID_DESCRIPTOR(ITF_NUM_LIGHTS, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_lights_report), EPNUM_LIGHTS,
                           CFG_TUD_HID_EP_BUFSIZE, 1)
    };

    uint8_t const* usb_handler::get_descriptor_configuration()
    {
        return desc_configuration;
    }

    static char const* string_desc_arr[] =
    {
        "Button 1",
        "Button 2",
        "Button 3",
        "Button 4",
        "Button 5",
        "Button 6",
        "Button 7",
        "Button 8",
        "Button 9",
        "Button 10",
        "Button 11",
        "Turntable R",
        "Turntable G",
        "Turntable B",
        "Centre Bar R",
        "Centre Bar G",
        "Centre Bar B"
    };

    char const* usb_handler::get_descriptor_string(uint8_t index)
    {
        if (index >= std::size(string_desc_arr)) return nullptr;

        return string_desc_arr[index];
    }
}

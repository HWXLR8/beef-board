#include "sdvx_usb.h"

#include "adc.h"
#include "analog_button.h"
#include "axis.h"
#include "beef.h"
#include "pins.h"
#include "sdvx_combo.h"
#include "sdvx_rgb.h"
#include "usb_descriptors.h"
#include "hardware/adc.h"

namespace SDVX
{
    hid_lights_t lights{};
    Axis* axis_x;
    Axis* axis_y;

    usb_handler::usb_handler()
    {
        adc_init();

        axis_x = new AnalogAxis(adc_gpio_pins[1]);
        axis_y = new AnalogAxis(adc_gpio_pins[0]);
        button_x = new AnalogButton(1, false);
        button_y = new AnalogButton(1, false);

        adc_dma_init();
    }

    void usb_handler::send_hid_report()
    {
        switch (config.sdvx_input_mode)
        {
        case InputMode::Joystick:
            {
                struct TU_ATTR_PACKED joystick_report_data_t
                {
                    uint8_t X;
                    uint8_t Y;
                    uint16_t Button; // bit-field representing which buttons have been pressed
                };
                static joystick_report_data_t report;

                report.X = axis_x->get();
                report.Y = axis_y->get();
                report.Button = button_state;

                tud_hid_report(REPORT_ID_JOYSTICK, &report, sizeof(report));
                break;
            }
        case InputMode::Keyboard:
            send_keyboard_report(config.sdvx_keys.key_codes, sizeof(config.sdvx_keys.key_codes));
            // mouse report will be sent inside hid_report_complete()
            break;
        default:
            break;
        }
    }

    void usb_handler::hid_set_report(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                                     uint8_t const* buffer, uint16_t bufsize)
    {
        if (report_id != REPORT_ID_JOYSTICK)
        {
            if (bufsize != sizeof(hid_lights_t) + 1)
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
    }

    void usb_handler::update()
    {
        axis_x->poll();
        axis_y->poll();
        button_x->poll(axis_x->get());
        button_y->poll(axis_y->get());
    }

    void usb_handler::update_lighting()
    {
    }

    combo_t usb_handler::get_button_combo()
    {
        return SDVX::get_button_combo();
    }

    uint16_t usb_handler::get_button_light_state()
    {
        return lights.buttons;
    }

    void usb_handler::on_combo_reset()
    {
    }

    void usb_handler::on_config_push()
    {
    }

    static constexpr tusb_desc_device_t desc_device = generate_device_descriptor(0x1CCF, 0x101C);

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
            // 7 physical (for some reason START is bound to B9 in EAC)
            HID_BUTTONS(9),

            // Button lighting
            HID_BUTTON_LIGHT(1),
            HID_BUTTON_LIGHT(2),
            HID_BUTTON_LIGHT(3),
            HID_BUTTON_LIGHT(4),
            HID_BUTTON_LIGHT(5),
            HID_BUTTON_LIGHT(6),
            HID_PADDING_OUTPUT(2),
            HID_BUTTON_LIGHT(7),
            HID_PADDING_OUTPUT(7),
        HID_COLLECTION_END,

        HID_REPORT_DESC_KEYBOARD,
        HID_REPORT_DESC_MOUSE
    };
    //@formatter:on

    const uint8_t* usb_handler::get_hid_descriptor_report(uint8_t instance)
    {
        (void)instance;
        return desc_hid_report;
    }

    void usb_handler::hid_report_complete(uint8_t instance, uint8_t const* report, uint16_t len)
    {
        auto is_hid_report = ITF_HID_BASE + instance == ITF_NUM_HID;
        if (!is_hid_report)
            return;

        if (report[0] == REPORT_ID_KEYBOARD && tud_hid_ready())
        {
            send_mouse_report(button_x->delta, button_y->delta);
        }
    }

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_HID_DESC_LEN)
    constexpr uint8_t desc_configuration[] =
    {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL-1, 0, CONFIG_TOTAL_LEN, 0x00, 500),

        // Interface number, string index, notification EP, notification EP size, EP Out & In address, EP Out & In size
        TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 0, EPNUM_CDC_CMD, 8, EPNUM_CDC, 0x80 | EPNUM_CDC, CFG_TUD_CDC_RX_BUFSIZE),

        // Interface number, string index, protocol, report descriptor len, EP Out & In address, size & polling interval
        TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID,
                                 0x80 | EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1),

        CONFIG_DESCRIPTOR
    };

    uint8_t const* usb_handler::get_descriptor_configuration()
    {
        return desc_configuration;
    }

    static char const* string_desc_arr[] =
    {
        "BT-A",
        "BT-B",
        "BT-C",
        "BT-D",
        "FX-L",
        "FX-R",
        "Start"
    };

    char const* usb_handler::get_descriptor_string(uint8_t index)
    {
        if (index >= std::size(string_desc_arr)) return nullptr;

        return string_desc_arr[index];
    }
}

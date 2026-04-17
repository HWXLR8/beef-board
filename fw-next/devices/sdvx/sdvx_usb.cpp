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
#include "hardware/dma.h"

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
        struct __attribute__((packed)) joystick_report_data_t
        {
            uint8_t X;
            uint8_t Y;
            uint16_t Button; // bit-field representing which buttons have been pressed
        };
        static joystick_report_data_t report;

        report.X = axis_x->get();
        report.Y = axis_y->get();
        report.Button = button_state;

        tud_hid_report(0, &report, sizeof(report));
    }

    void usb_handler::hid_set_report(uint8_t itf, uint8_t report_id, hid_report_type_t report_type,
                                     uint8_t const* buffer, uint16_t bufsize)
    {
        assert(bufsize == sizeof(hid_lights_t));
        memcpy(&lights, buffer, bufsize);
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

    static constexpr tusb_desc_device_t desc_device =
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0x1CCF,
        .idProduct = 0x101C,
        .bcdDevice = 0x0100,

        .iManufacturer = 0x01,
        .iProduct = 0x02,

        .bNumConfigurations = 0x01
    };

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
            HID_USAGE(HID_USAGE_DESKTOP_POINTER),
            HID_COLLECTION(HID_COLLECTION_LOGICAL),
                HID_USAGE(HID_USAGE_DESKTOP_X),
                HID_USAGE(HID_USAGE_DESKTOP_Y),
                HID_LOGICAL_MIN_N(0, 2),
                HID_LOGICAL_MAX_N(255, 2),
                HID_PHYSICAL_MIN(0),
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
        HID_COLLECTION_END
    };
    //@formatter:on

    const uint8_t* usb_handler::get_hid_descriptor_report()
    {
        return desc_hid_report;
    }

    constexpr uint8_t desc_configuration[] =
    {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 500),

        // Interface number, string index, notification EP, notification EP size, EP Out & In address, EP Out & In size
        TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 0, EPNUM_CDC_CMD, 8, EPNUM_CDC, 0x80 | EPNUM_CDC, 64),

        // Interface number, string index, protocol, report descriptor len, EP Out & In address, size & polling interval
        TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID,
                                 0x80 | EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1)
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

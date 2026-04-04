#include "iidx_usb.h"

#include "analog_button.h"
#include "axis.h"
#include "combo.h"
#include "config.h"
#include "iidx_rgb.h"

namespace IIDX
{
    hid_lights_t lights{};
    int8_t tt1_report = 0;

    usb_handler::usb_handler()
    {
        RgbManager::init();
        button_x = new AnalogButton(config.tt_deadzone, true);
    }

    void usb_handler::send_hid_report()
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

    void usb_handler::hid_set_report(uint8_t itf, uint8_t report_id, hid_report_type_t report_type,
                                     uint8_t const* buffer, uint16_t bufsize)
    {
        assert(bufsize == sizeof(hid_lights_t));
        memcpy(&lights, buffer, bufsize);
    }

    void usb_handler::update()
    {
        tt_x.poll();
        tt1_report = button_x->poll(tt_x.get());

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
}

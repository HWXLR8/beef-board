#pragma once

#include "beef.h"
#include "usb_handler.h"

namespace IIDX
{
    class usb_handler : public ::usb_handler
    {
    public:
        usb_handler();

        void send_hid_report() override;
        void hid_set_report(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                            uint16_t bufsize) override;
        void update() override;
        void update_lighting() override;
        combo_t get_button_combo() override;
        uint16_t get_button_light_state() override;
        void on_combo_reset() override;
        void on_config_push() override;
    };

    enum
    {
        BUTTON_TT_NEG = 1 << 11,
        BUTTON_TT_POS = 1 << 12,
        MAIN_BUTTONS_ALL = BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4 | BUTTON_5 | BUTTON_6 | BUTTON_7,
        EFFECTORS_ALL = BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11
    };
}

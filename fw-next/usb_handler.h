#pragma once

#include "combo.h"

class usb_handler
{
public:
    virtual ~usb_handler() = default;

    virtual void send_hid_report() = 0;
    virtual void hid_set_report(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                                uint16_t bufsize) = 0;
    virtual void update() = 0;
    virtual void update_lighting() = 0;
    virtual void on_config_push() = 0;
    virtual combo_t get_button_combo() = 0;
    virtual void on_combo_reset() = 0;
    virtual uint16_t get_button_light_state() = 0;
};

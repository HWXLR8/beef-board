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

    virtual tusb_desc_device_t const* get_descriptor_device() = 0;
    virtual const uint8_t* get_hid_descriptor_report() = 0;
    virtual uint8_t const* get_descriptor_configuration() = 0;
    virtual char const* get_descriptor_string(uint8_t index) = 0;
};

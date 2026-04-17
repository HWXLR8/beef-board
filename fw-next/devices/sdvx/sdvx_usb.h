#pragma once

#include "combo.h"
#include "usb_handler.h"

namespace SDVX
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

        tusb_desc_device_t const* get_descriptor_device() override;
        const uint8_t* get_hid_descriptor_report() override;
        uint8_t const* get_descriptor_configuration() override;
        char const* get_descriptor_string(uint8_t index) override;
    };
}

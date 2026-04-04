#pragma once

#include "iidx_combo.h"

namespace IIDX
{
    struct __attribute__((packed)) hid_lights_t
    {
        uint16_t buttons = 0;
        rgb_t tt_lights;
        rgb_t bar_lights;
    };

    namespace RgbManager
    {
        namespace Turntable
        {
            void reverse_tt(bool reverse_tt);
            void display_tt_change(const rgb_t &colour,
                                   uint8_t value,
                                   uint8_t range);
        }

        void init();
        void update(int8_t tt1_report, const hid_lights_t &led_state_from_hid_report);
    }
}

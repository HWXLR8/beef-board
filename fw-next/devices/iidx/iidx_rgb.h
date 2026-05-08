#pragma once

#include "iidx_combo.h"
#include "iidx_defs.h"

namespace IIDX
{
    struct TU_ATTR_PACKED hid_lights_t
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

        namespace Bar
        {
            extern rgb_t tape_leds[LIGHT_BAR_LEDS];
        }

        void init();
        void update(int8_t tt1_report, const hid_lights_t &led_state_from_hid_report);
    }
}

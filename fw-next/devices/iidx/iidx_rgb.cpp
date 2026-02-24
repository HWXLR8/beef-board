#include "iidx_rgb.h"

#include "beef.h"
#include "config.h"
#include "iidx_combo.h"
#include "rgb.h"
#include "ws2812.h"

namespace IIDX::RgbManager
{
    namespace Turntable
    {
        void update(int8_t tt1_report, const rgb_t &lights)
        {
            // Ignore turntable effect if notifying a setting change
            if (combo_timer.is_active())
            {
                return;
            }

            switch (config.tt_effect)
            {
            case TurntableMode::HID:
                hid(tt_leds.begin(), tt_leds.end(), lights);
                break;
            default:
                break;
            }
        }
    }

    namespace Bar
    {
        enum PlayerSide
        {
            P1,
            P2
        };

        void update(const rgb_t &lights)
        {
            switch (config.bar_effect)
            {
            case BarMode::HID:
                hid(bar_leds.begin(), bar_leds.end(), lights);
                break;
            default:
                break;
            }
        }
    }

    void update(const int8_t tt1_report,
                const hid_lights_t &led_state_from_hid_report)
    {
        Turntable::update(tt1_report,
                          led_state_from_hid_report.tt_lights);

        Bar::update(led_state_from_hid_report.bar_lights);
    }
}

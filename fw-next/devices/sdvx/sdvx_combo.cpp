#include "sdvx_combo.h"

#include "beef.h"

namespace SDVX
{
    enum
    {
        DISABLE_LEDS = BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4 | BUTTON_9
    };

    combo_t get_button_combo()
    {
        switch (button_state)
        {
        case DISABLE_LEDS:
            return {
                .update_config = toggle_disable_leds
            };
        default:
            return {};
        }
    }

    void on_combo_reset()
    {
    }
}

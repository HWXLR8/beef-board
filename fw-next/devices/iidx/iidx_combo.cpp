#include "iidx_combo.h"
#include "beef.h"
#include "iidx_usb.h"

namespace IIDX
{
    timer_t combo_timer;

    enum
    {
        REVERSE_TT = BUTTON_1 | BUTTON_7 | BUTTON_8,
        TT_EFFECTS = BUTTON_2 | BUTTON_8 | BUTTON_11,
        TT_DEADZONE_INCR = BUTTON_7 | BUTTON_8 | BUTTON_11,
        TT_DEADZONE_DECR = BUTTON_5 | BUTTON_8 | BUTTON_11,
        TT_RATIO_INCR = BUTTON_1 | BUTTON_8 | BUTTON_11,
        TT_RATIO_DECR = BUTTON_3 | BUTTON_8 | BUTTON_11,
        BAR_EFFECTS = BUTTON_6 | BUTTON_8 | BUTTON_10,
        DISABLE_LEDS = BUTTON_4 | BUTTON_8 | BUTTON_11,
        TT_HSV_HUE = BUTTON_2 | BUTTON_11,
        TT_HSV_SAT = BUTTON_4 | BUTTON_11,
        TT_HSV_VAL = BUTTON_6 | BUTTON_11,
    };

    combo_t ComboProcessor::get_button_combo()
    {
        // Ignore digital TT inputs
        switch (button_state & ~(BUTTON_TT_NEG | BUTTON_TT_POS))
        {
        case REVERSE_TT:
            return {
                .update_config = toggle_reverse_tt
            };
        case TT_EFFECTS:
            return {
                .update_config = cycle_tt_effects
            };
        case TT_DEADZONE_INCR:
            return {
                .update_config = increase_deadzone
            };
        case TT_DEADZONE_DECR:
            return {
                .update_config = decrease_deadzone
            };
        case TT_RATIO_INCR:
            return {
                .update_config = increase_ratio
            };
        case TT_RATIO_DECR:
            return {
                .update_config = decrease_ratio
            };
        case BAR_EFFECTS:
            return {
                .update_config = cycle_bar_effects
            };
        case DISABLE_LEDS:
            return {
                .update_config = toggle_disable_leds
            };
        case TT_HSV_HUE:
            return {
                .continuous = true,
                .update_config = tt_hsv_set_hue,
            };
        case TT_HSV_SAT:
            return {
                .continuous = true,
                .update_config = tt_hsv_set_sat,
            };
        case TT_HSV_VAL:
            return {
                .continuous = true,
                .update_config = tt_hsv_set_val,
            };
        default:
            return {};
        }
    }

    void ComboProcessor::on_reset()
    {
        combo_timer.reset();
    }
}

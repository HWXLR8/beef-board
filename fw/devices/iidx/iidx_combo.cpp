#include "iidx_combo.h"
#include "iidx_usb.h"

namespace Iidx {
  enum {
    REVERSE_TT       = BUTTON_1 | BUTTON_7 | BUTTON_8,
    TT_EFFECTS       = BUTTON_2 | BUTTON_8 | BUTTON_11,
    TT_DEADZONE_INCR = BUTTON_7 | BUTTON_8 | BUTTON_11,
    TT_DEADZONE_DECR = BUTTON_5 | BUTTON_8 | BUTTON_11,
    TT_RATIO_INCR    = BUTTON_1 | BUTTON_8 | BUTTON_11,
    TT_RATIO_DECR    = BUTTON_3 | BUTTON_8 | BUTTON_11,
    BAR_EFFECTS      = BUTTON_6 | BUTTON_8 | BUTTON_10,
    DISABLE_LED      = BUTTON_4 | BUTTON_8 | BUTTON_11,
    TT_HSV_HUE       = BUTTON_2 | BUTTON_11,
    TT_HSV_SAT       = BUTTON_4 | BUTTON_11,
    TT_HSV_VAL       = BUTTON_6 | BUTTON_11,
  };

  combo get_button_combo(uint16_t button_state) {
    // Ignore digital TT inputs
    button_state &= ~(BUTTON_TT_NEG | BUTTON_TT_POS);

    switch (button_state) {
      case REVERSE_TT:
        return {
            .config_set = toggle_reverse_tt
        };
      case TT_EFFECTS:
        return {
            .config_set = cycle_tt_effects
        };
      case TT_DEADZONE_INCR:
        return {
            .config_set = increase_deadzone
        };
      case TT_DEADZONE_DECR:
        return {
            .config_set = decrease_deadzone
        };
      case TT_RATIO_INCR:
        return {
            .config_set = increase_ratio
        };
      case TT_RATIO_DECR:
        return {
            .config_set = decrease_ratio
        };
      case BAR_EFFECTS:
        return {
            .config_set = cycle_bar_effects
        };
      case DISABLE_LED:
        return {
            .config_set = toggle_disable_led
        };
      case TT_HSV_HUE:
        return {
            .continuous = true,
            .config_set = tt_hsv_set_hue,
        };
      case TT_HSV_SAT:
        return {
            .continuous = true,
            .config_set = tt_hsv_set_sat,
        };
      case TT_HSV_VAL:
        return {
            .continuous = true,
            .config_set = tt_hsv_set_val,
        };
      default:
        return {};
    }
  }
}

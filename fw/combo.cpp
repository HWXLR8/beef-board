#include "beef.h"
#include "combo.h"
#include "rgb_manager.h"

struct combo {
  bool continuous;
  callback (*config_set)(config*);
};

combo get_button_combo() {
  // Ignore digital TT inputs
  const auto state = static_cast<Combo>(
    button_state & ~(BUTTON_TT_NEG | BUTTON_TT_POS));

  switch (state) {
    case Combo::REVERSE_TT:
      return {
        .config_set = toggle_reverse_tt
      };
    case Combo::TT_EFFECTS:
      return {
        .config_set = cycle_tt_effects
      };
    case Combo::TT_DEADZONE_INCR:
      return {
        .config_set = increase_deadzone
      };
    case Combo::TT_DEADZONE_DECR:
      return {
        .config_set = decrease_deadzone
      };
    case Combo::TT_RATIO_INCR:
      return {
        .config_set = increase_ratio
      };
    case Combo::TT_RATIO_DECR:
      return {
        .config_set = decrease_ratio
      };
    case Combo::BAR_EFFECTS:
      return {
        .config_set = cycle_bar_effects
      };
    case Combo::DISABLE_LED:
      return {
        .config_set = toggle_disable_led
      };
    case Combo::TT_HSV_HUE:
      return {
        .continuous = true,
        .config_set = tt_hsv_set_hue,
      };
    case Combo::TT_HSV_SAT:
      return {
        .continuous = true,
        .config_set = tt_hsv_set_sat,
      };
    case Combo::TT_HSV_VAL:
      return {
        .continuous = true,
        .config_set = tt_hsv_set_val,
      };
    default:
      return {};
  }
}

void process_combos(config* current_config,
                    timer* combo_timer,
                    timer* combo_lights_timer) {
  static bool combo_activated = false;
  static bool ignore_combo = false;
  static bool in_hid = false;
  static callback update_callback;

  const auto button_combo = get_button_combo();
  if (button_combo.config_set != nullptr) {
    if (!combo_activated) {
      combo_activated = true;
      in_hid = !reactive_led;
    }
    reactive_led = true;

    if (ignore_combo)
      return;

    if (button_combo.continuous) {
      update_callback = button_combo.config_set(current_config);
      if (update_callback.addr == nullptr) {
        // Invalid combo held
        timer_arm(combo_lights_timer, 500);
      }
    } else {
      // arm timer if not already armed
      if (!timer_is_armed(combo_timer))
        timer_arm(combo_timer, 1000);

      if (timer_check_if_expired_reset(combo_timer)) {
        update_callback = button_combo.config_set(current_config);
        timer_arm(combo_lights_timer, CONFIG_CHANGE_NOTIFY_TIME);
        ignore_combo = true;
      }
    }

    return;
  }

  if (combo_activated) {
    combo_activated = false;
    ignore_combo = false;
    if (in_hid)
      reactive_led = false;

    timer_init(combo_timer);
    timer_init(combo_lights_timer);
    timer_init(&RgbManager::Turntable::combo_timer);

    if (update_callback.addr != nullptr)
      config_update(update_callback.addr, update_callback.val);
  }
}

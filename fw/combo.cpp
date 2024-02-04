#include "beef.h"
#include "combo.h"

struct combo {
  uint16_t button_combo;
  bool continuous;
  void (*config_set)(config*);
  void (*config_update)(config*);
};

combo button_combos[NUM_OF_COMBOS] = {
  {
    button_combo: REVERSE_TT_COMBO,
    config_set: toggle_reverse_tt
  },
  {
    button_combo: TT_EFFECTS_COMBO,
    config_set: cycle_tt_effects
  },
  {
    button_combo: TT_DEADZONE_INCR_COMBO,
    config_set: increase_deadzone
  },
  {
    button_combo: TT_DEADZONE_DECR_COMBO,
    config_set: decrease_deadzone
  },
  {
    button_combo: BAR_EFFECTS_COMBO,
    config_set: cycle_bar_effects
  },
  {
    button_combo: DISABLE_LED_COMBO,
    config_set: toggle_disable_led
  },
  {
    button_combo: TT_HSV_HUE_COMBO,
    continuous: true,
    config_set: tt_hsv_set_hue,
    config_update: tt_hsv_update_hue
  },
  {
    button_combo: TT_HSV_SAT_COMBO,
    continuous: true,
    config_set: tt_hsv_set_sat,
    config_update: tt_hsv_update_sat
  },
  {
    button_combo: TT_HSV_VAL_COMBO,
    continuous: true,
    config_set: tt_hsv_set_val,
    config_update: tt_hsv_update_val
  },
};

void process_combos(config* current_config,
                    timer* combo_timer,
                    timer* combo_lights_timer) {
  static bool combo_activated = false;
  static bool ignore_combo = false;
  static bool in_hid = false;
  static combo last_combo;

  for (uint8_t i = 0; i < NUM_OF_COMBOS; ++i) {
    auto button_combo = button_combos[i];
    if (is_pressed_strict(button_combo.button_combo)) {
      last_combo = button_combo;

      if (!combo_activated) {
        combo_activated = true;
        in_hid = !reactive_led;
      }
      if (in_hid) {
        reactive_led = true; 
      }

      if (ignore_combo) {
        return;
      }

      if (button_combo.continuous) {
        button_combo.config_set(current_config);
      } else {
        // arm timer if not already armed
        if (!timer_is_armed(combo_timer)) {
          timer_arm(combo_timer, 1000);
        }

        if (timer_check_if_expired_reset(combo_timer)) {
          button_combo.config_set(current_config);
          timer_arm(combo_lights_timer, CONFIG_CHANGE_NOTIFY_TIME);
          ignore_combo = true;
        }
      }

      return;
    }
  }

  if (combo_activated) {
    combo_activated = false;
    ignore_combo = false;
    if (in_hid) {
      reactive_led = false;
    }

    timer_init(combo_timer);
    timer_init(combo_lights_timer);
    timer_init(&RgbManager::Turntable::combo_timer);

    if (last_combo.continuous) {
      last_combo.config_update(current_config);
    }
  }
}

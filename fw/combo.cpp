#include "beef.h"
#include "combo.h"
#include "config.h"
#include "hid.h"
#include "rgb_helper.h"

combo (*get_button_combo_callback) (uint16_t);
timer combo_lights_timer;

void process_combos() {
  static timer combo_timer;
  static bool combo_activated = false;
  static bool ignore_combo = false;
  static callback config_update_callback;

  const auto button_combo = get_button_combo_callback(button_state);
  if (button_combo.config_set != nullptr) {
    combo_activated = true;
    reactive_led = true;

    if (ignore_combo)
      return;

    if (button_combo.continuous) {
      config_update_callback = button_combo.config_set(&current_config);
      if (config_update_callback.addr == nullptr) {
        // Invalid combo held
        timer_arm(&combo_lights_timer, 500);
      }
    } else {
      // arm timer if not already armed
      if (!timer_is_armed(&combo_timer))
        timer_arm(&combo_timer, 1000);

      if (timer_check_if_expired_reset(&combo_timer)) {
        config_update_callback = button_combo.config_set(&current_config);
        timer_arm(&combo_lights_timer, CONFIG_CHANGE_NOTIFY_TIME);
        ignore_combo = true;
      }
    }

    return;
  }

  if (combo_activated) {
    combo_activated = false;
    ignore_combo = false;

    timer_reset(&combo_timer);
    timer_reset(&combo_lights_timer);
    timer_reset(&RgbHelper::combo_timer);

    if (config_update_callback.addr != nullptr)
      config_update(config_update_callback.addr, config_update_callback.val);
  }
}

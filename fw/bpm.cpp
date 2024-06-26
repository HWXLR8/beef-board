#include <FastLED/src/FastLED.h>

#include "bpm.h"
#include "rgb_helper.h"

#define DECAY_TIME 30 * BAR_ANIM_NORMALISE
// This effectively controls how easy it is to reach max_level
#define BUTTON_GUARD_TIME 15 * BAR_ANIM_NORMALISE

Bpm::Bpm(const uint8_t max_level) : max_level(max_level){}

uint8_t Bpm::update(const uint16_t button_state) {
  EVERY_N_MILLIS(1) {
    if (!timer_is_active(&button_guard)) {
      const bool pressed_or_released = button_state ^ last_button_state;
      if (pressed_or_released) {
        current_level++;
        timer_arm(&decay, DECAY_TIME);
        timer_arm(&button_guard, BUTTON_GUARD_TIME);
      }
    }

    if (current_level > max_level)
      current_level = max_level;

    if (current_level > 0 && timer_is_expired(&decay)) {
      current_level--;
      timer_arm(&decay, DECAY_TIME);
    }

    last_button_state = button_state;
  }

  return current_level;
}

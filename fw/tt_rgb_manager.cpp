#include "tt_rgb_manager.h"

timer led_timer;
timer combo_tt_led_timer;
CRGB tt_leds[RING_LIGHT_LEDS];
timer spin_timer;

void set_tt_leds(rgb_light lights) {
  fill_solid(tt_leds, RING_LIGHT_LEDS,
             CRGB(lights.r, lights.g, lights.b));
}

void set_tt_leds_blue(void) {
  fill_solid(tt_leds, RING_LIGHT_LEDS, CRGB::Blue);
}

void set_tt_leds_red(void) {
  fill_solid(tt_leds, RING_LIGHT_LEDS, CRGB::Red);
}

void set_tt_leds_off(void) {
  fill_solid(tt_leds, RING_LIGHT_LEDS, CRGB::Black);
}

void react_to_scr(int8_t tt_report) {
  if (tt_report == 1) {
    set_tt_leds_blue();
    timer_arm(&led_timer, 500);
  } else if (tt_report == -1) {
    set_tt_leds_red();
    timer_arm(&led_timer, 500);
  }
  if (!timer_is_active(&led_timer)) {
    set_tt_leds_off();
  }
}

void spin(void) {
  static uint8_t spin_counter = 0;
  if (timer_check_if_expired_reset(&spin_timer)) {
    spin_counter = (spin_counter + 1) % RING_LIGHT_LEDS;
    for (int i = 0; i < RING_LIGHT_LEDS; i++) {
      if (i == spin_counter) {
        tt_leds[i] = CRGB::Blue;
      } else {
        tt_leds[i] = CRGB::Black;
      }
    }

    timer_arm(&spin_timer, 50);
  }
}

// tt +1 is counter-clockwise, -1 is clockwise
void tt_rgb_manager_update(int8_t tt_report,
                           rgb_light lights,
                           ring_light_mode mode) {
  // Ignore turtable effect if notifying a mode change
  if (!timer_is_active(&combo_tt_led_timer)) {
    switch(mode) {
      case SPIN:
        spin();
        break;
      case REACT_TO_SCR:
        react_to_scr(tt_report);
        break;
      case HID:
        set_tt_leds(lights);
        break;
      default:
        break;
    }
  }
}

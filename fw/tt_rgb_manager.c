#include "tt_rgb_manager.h"

void rgb(struct cRGB* led, uint8_t r, uint8_t g, uint8_t b) {
  led->r = r;
  led->g = g;
  led->b = b;
}

void set_tt_leds(rgb_lights lights) {
  for (int i = 0; i < RING_LIGHT_LEDS; ++i) {
    rgb(&led[i], lights.r, lights.g, lights.b);
  }
  ws2812_setleds(led, RING_LIGHT_LEDS);
}

void set_led_blue(void) {
  set_tt_leds((rgb_lights){0, 0, 255});
}

void set_led_red(void) {
  set_tt_leds((rgb_lights){255, 0, 0});
}

void set_led_off(void) {
  set_tt_leds((rgb_lights){0});
}

void react_to_scr(int8_t tt_report) {
  if (tt_report == 1) {
    set_led_blue();
    timer_arm(&led_timer, 500);
  } else if (tt_report == -1) {
    set_led_red();
    timer_arm(&led_timer, 500);
  } 
  if (timer_check_if_expired_reset(&led_timer) || !timer_is_armed(&led_timer)) {
    set_led_off();
  }
}

void spin(void) {
  static uint8_t spin_counter = 0;
  if (timer_check_if_expired_reset(&spin_timer)) {
    rgb(&(led[spin_counter]), 0, 0, 0);
    spin_counter = (spin_counter + 1) % RING_LIGHT_LEDS;
    rgb(&(led[spin_counter]), 0, 0, 255);
    ws2812_setleds(led, RING_LIGHT_LEDS);

    timer_arm(&spin_timer, 50);
  }
}

// tt +1 is counter-clockwise, -1 is clockwise
void tt_rgb_manager_update(int8_t tt_report,
                           rgb_lights lights,
                           ring_light_mode mode) {
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

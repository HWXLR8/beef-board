#include "beef.h"

timer led_timer;
struct cRGB led[RING_LIGHT_LEDS];

void set_led_blue() {
  led[0].r = 0; led[0].g = 0; led[0].b = 255;
  led[11].r = 0; led[11].g = 0; led[11].b = 255;
  ws2812_setleds(led, RING_LIGHT_LEDS);
}

void set_led_red() {
  led[0].r = 255; led[0].g = 0; led[0].b = 0;
  led[11].r = 255; led[11].g = 0; led[11].b = 0;
  ws2812_setleds(led, RING_LIGHT_LEDS);
}

void set_led_off() {
  for (int i = 0; i < RING_LIGHT_LEDS; ++i) {
    led[i].r = 0; led[i].g = 0; led[i].b = 0;
  }
  ws2812_setleds(led, RING_LIGHT_LEDS);
}

void react_to_scr(int8_t tt_report) {
  if (tt_report == 1) {
    set_led_blue();
    timer_arm(&led_timer, 500);
  } else if (tt_report == -1) {
    set_led_red();
    timer_arm(&led_timer, 500);
  } 
  if (timer_check_if_expired_reset(&led_timer)) {
    set_led_off();
  }
}

void rgb(struct cRGB* led, uint8_t r, uint8_t g, uint8_t b) {
  led->r = r;
  led->g = g;
  led->b = b;
}

uint8_t spin_counter = 0;
timer spin_timer;
void spin() {
  if (timer_check_if_expired_reset(&spin_timer)) {
    rgb(&(led[spin_counter]), 0, 0, 0);
    rgb(&(led[(spin_counter + 1) % RING_LIGHT_LEDS]), 0, 0, 255);
    ws2812_setleds(led, RING_LIGHT_LEDS);

    timer_arm(&spin_timer, 50);

    spin_counter += 1;
    spin_counter %= RING_LIGHT_LEDS;
  }
}

// tt +1 is counter-clockwise, -1 is clockwise
void tt_rgb_manager_update(int8_t tt_report) {
  switch(ring_light_mode) {
    case REACT_TO_SCR:
      react_to_scr(tt_report);
      break;
    case SPIN:
      spin();
      break;
  }
}

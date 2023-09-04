#include "beef.h"

timer led_timer;
struct cRGB led[RING_LIGHT_LEDS];

void set_led_blue() {
  led[0].r = 0; led[0].g = 0; led[0].b = 255;
  led[11].r = 0; led[11].g = 0; led[11].b = 255;
  ring_light_setleds(led, RING_LIGHT_LEDS);
}

void set_led_red() {
  led[0].r = 255; led[0].g = 0; led[0].b = 0;
  led[11].r = 255; led[11].g = 0; led[11].b = 0;
  ring_light_setleds(led, RING_LIGHT_LEDS);
}

void set_led_off() {
  for (int i = 0; i < RING_LIGHT_LEDS; ++i) {
    led[i].r = 0; led[i].g = 0; led[i].b = 0;
  }
  ring_light_setleds(led, RING_LIGHT_LEDS);
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
  if (timer_check_if_expired_rearm(&spin_timer, 50)) {
    rgb(&(led[spin_counter]), 0, 0, 0);
    rgb(&(led[(spin_counter + 1) % RING_LIGHT_LEDS]), 0, 0, 255);
    ring_light_setleds(led, RING_LIGHT_LEDS);

    //timer_arm(&spin_timer, 50);

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

struct cRGB light_bar_leds[LIGHT_BAR_LEDS];
timer flash_timer;
bool flash_on = false;

// toggle flash every 500ms
void flash() {
  if (timer_check_if_expired_rearm(&flash_timer, 500)) {
    if (flash_on) {
      for (int i = 0; i < LIGHT_BAR_LEDS; ++i) {
        rgb(&(light_bar_leds[i]), 0, 0, 0);
      }
      light_bar_setleds(light_bar_leds, LIGHT_BAR_LEDS);
      flash_on = false;
    } else {
      for (int i = 0; i < LIGHT_BAR_LEDS; ++i) {
        rgb(&(light_bar_leds[i]), 0, 0, 255);
      }
      light_bar_setleds(light_bar_leds, LIGHT_BAR_LEDS);
      flash_on = true;
    }
    timer_arm(&flash_timer, 500);
  }
}

void idle() {
  for (int i = 0; i < LIGHT_BAR_LEDS; ++i) {
    rgb(&(light_bar_leds[i]), 0, 255, 0);
  }
  light_bar_setleds(light_bar_leds, LIGHT_BAR_LEDS);
}

void light_bar_update() {
  switch(light_bar_mode) {
    case FLASH:
      flash();
      break;
    case IDLE:
      idle();
      break;
    case DISABLE:
      break;
  }
}

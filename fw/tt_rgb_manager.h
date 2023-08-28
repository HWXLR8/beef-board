#include "beef.h"

timer led_timer;

void set_led_blue() {
    led[0].r = 0; led[0].g = 0; led[0].b = 255;
    ws2812_setleds(led, 60);
}

void set_led_red() {
    led[0].r = 255; led[0].g = 0; led[0].b = 0;
    ws2812_setleds(led, 60);
}

void set_led_off() {
    led[0].r = 0; led[0].g = 0; led[0].b = 0;
    ws2812_setleds(led, 60);
}


// tt +1 is counter-clockwise, -1 is clockwise
void tt_rgb_manager_update(int8_t tt_report) {
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

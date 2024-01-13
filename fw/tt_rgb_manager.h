#pragma once

#include "rgb.h"
#include "timer.h"
#include "ws2812.h"

timer led_timer;
timer spin_timer;

// tentative names for LED ring modes
enum ring_light_mode {
  SPIN,
  REACT_TO_SCR
};

enum ring_light_mode ring_light_mode;

void set_tt_leds(rgb_lights lights);
void rgb(struct cRGB* led, uint8_t r, uint8_t g, uint8_t b);
void tt_rgb_manager_update(int8_t tt_report);

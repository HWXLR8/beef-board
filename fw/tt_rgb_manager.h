#pragma once

#include "FastLED/src/FastLED.h"

#include "config.h"
#include "rgb.h"
#include "timer.h"

extern timer led_timer;
extern timer combo_tt_led_timer;
extern CRGB tt_leds[RING_LIGHT_LEDS];
extern timer spin_timer;

void set_tt_leds(rgb_light lights);
void set_tt_leds_blue(void);
void set_tt_leds_red(void);
void set_tt_leds_off(void);

void react_to_scr(int8_t tt_report);
void spin(void);

void tt_rgb_manager_update(int8_t tt_report,
                           rgb_light lights,
                           ring_light_mode mode);

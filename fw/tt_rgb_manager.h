#pragma once

#include "config.h"
#include "rgb.h"
#include "timer.h"
#include "ws2812.h"

timer led_timer;
timer combo_tt_led_timer;
struct cRGB tt_leds[RING_LIGHT_LEDS];
timer spin_timer;

void rgb(struct cRGB* led, uint8_t r, uint8_t g, uint8_t b);
void set_tt_leds(rgb_light lights);
void set_tt_leds_blue(void);
void set_tt_leds_red(void);
void set_tt_leds_off(void);

void react_to_scr(int8_t tt_report);
void spin(void);

void tt_rgb_manager_update(int8_t tt_report,
                           rgb_light lights,
                           ring_light_mode mode);

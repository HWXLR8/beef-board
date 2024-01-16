#pragma once

#include "config.h"
#include "rgb.h"
#include "timer.h"
#include "ws2812.h"

timer led_timer;
struct cRGB led[RING_LIGHT_LEDS];
timer spin_timer;

void rgb(struct cRGB* led, uint8_t r, uint8_t g, uint8_t b);
void set_tt_leds(rgb_lights lights);
void set_led_blue(void);
void set_led_red(void);
void set_led_off(void);

void react_to_scr(int8_t tt_report);
void spin(void);

void tt_rgb_manager_update(int8_t tt_report,
                           rgb_lights lights,
                           ring_light_mode mode);

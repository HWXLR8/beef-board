#pragma once

#include <FastLED/src/FastLED.h>

#include "rgb.h"
#include "rgb_patterns.h"

// Keep these as #defines to make sure order of operations is correct
#if RING_LIGHT_LEDS > 0
#define RING_ANIM_NORMALISE 24 / RING_LIGHT_LEDS
#else
#define RING_ANIM_NORMALISE 1
#endif
#if LIGHT_BAR_LEDS > 0
#define BAR_ANIM_NORMALISE 16 / LIGHT_BAR_LEDS
#else
#define BAR_ANIM_NORMALISE 1
#endif

constexpr auto DEFAULT_COLOUR = HSV{ 128, 255, 255 }; // Aqua
extern CRGB tt_leds[RING_LIGHT_LEDS];
extern CRGB bar_leds[LIGHT_BAR_LEDS];

struct rgb_light {
  uint8_t r, g, b;
} ATTR_PACKED;

namespace RgbHelper {
  extern timer combo_timer;

  void init();

  bool set_rgb(CRGB* leds, const uint8_t n, const rgb_light &lights);
  bool set_rgb(CRGB* leds, const uint8_t n, const CRGB &rgb);
  bool set_hsv(CRGB* leds, const uint8_t n, const HSV &hsv);
  bool breathing(BreathingPattern &breathing_pattern,
                 CRGB* leds, const uint8_t n, const HSV &hsv);
  bool hid(CRGB* leds, const uint8_t n, rgb_light lights);

  bool ready_to_present();
  void show_tt();
  void show_bar();
}

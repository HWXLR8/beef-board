#pragma once

#include <FastLED/src/FastLED.h>

#include "rgb.h"
#include "rgb_patterns.h"

#if !defined(RING_LIGHT_LEDS)
  #define RING_LIGHT_LEDS 24
#endif
#if !defined(LIGHT_BAR_LEDS)
  #define LIGHT_BAR_LEDS 16
#endif
#define RING_ANIM_NORMALISE RING_LIGHT_LEDS / 24
#define BAR_ANIM_NORMALISE LIGHT_BAR_LEDS / 16

constexpr auto DEFAULT_COLOUR = HSV{ 128, 255, 255 }; // Aqua
extern CRGB tt_leds[RING_LIGHT_LEDS];
extern CRGB bar_leds[LIGHT_BAR_LEDS];

namespace RgbHelper {
  void init();

  void set_rgb(CRGB* leds, const uint8_t n, const rgb_light &lights);
  void set_hsv(CRGB* leds, const uint8_t n, const HSV &hsv);
  void breathing(BreathingPattern &breathing_pattern,
                 CRGB* leds, const uint8_t n, const HSV &hsv);
  void hid(CRGB* leds, const uint8_t n, rgb_light lights);
  bool ready_to_present();
}

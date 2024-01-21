#pragma once

#include "FastLED/src/FastLED.h"

#include "rgb.h"
#include "timer.h"

#define RING_LIGHT_LEDS 24
#define LIGHT_BAR_LEDS 16

namespace RgbManager {
  namespace Turntable {
    extern timer combo_timer;
    extern CRGB leds[RING_LIGHT_LEDS];

    // tentative names for LED ring modes
    enum Mode {
      PLACEHOLDER1, // single colour
      SPIN,
      PLACEHOLDER2, // colour shift
      PLACEHOLDER3, // static rainbow
      PLACEHOLDER4, // reactive rainbow
      REACT_TO_SCR,
      PLACEHOLDER5, // breathing
      HID,
      DISABLE,
      COUNT
    };
    static_assert(sizeof(Mode) == sizeof(uint8_t));

    void init();
    void set_leds(rgb_light lights);
    void set_leds_off(void);
    void reverse_tt(uint8_t reverse_tt);
    void display_tt_change(uint8_t deadzone, int range);
    void update(int8_t tt_report,
                rgb_light lights,
                Mode mode);
  }

  namespace Bar {
    extern CRGB leds[LIGHT_BAR_LEDS];

    enum Mode {
      PLACEHOLDER1, // beat
      PLACEHOLDER2, // reactive p1
      PLACEHOLDER3, // audio spectrum
      PLACEHOLDER4, // reactive p2
      HID,
      COUNT
    };
    static_assert(sizeof(Mode) == sizeof(uint8_t));

    void init();
    void set_leds(rgb_light lights);
    void set_leds_off(void);
    void update(rgb_light lights,
                Mode mode);
  }
}

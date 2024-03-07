#pragma once

#include "FastLED/src/FastLED.h"

#include "rgb.h"
#include "timer.h"

#define RING_LIGHT_LEDS 24
#define LIGHT_BAR_LEDS 16

const auto default_colour = HSV{ 127, 255, 255 }; // Aqua

namespace RgbManager {
  namespace Turntable {
    extern timer combo_timer;

    // tentative names for LED ring modes
    enum class Mode : uint8_t {
      STATIC,
      SPIN,
      SHIFT,
      RAINBOW_STATIC,
      RAINBOW_REACT,
      RAINBOW_SPIN,
      REACT_TO_SCR,
      BREATHING,
      HID,
      DISABLE,
      COUNT
    };

    void init();
    void set_rgb(rgb_light lights);
    void set_hsv(HSV hsv);
    void set_leds_off();
    void reverse_tt(uint8_t reverse_tt);
    void display_tt_change(uint8_t deadzone, int range);
    void update(int8_t tt_report,
                rgb_light lights,
                HSV hsv,
                Mode mode);
  }

  namespace Bar {
    enum class Mode : uint8_t {
      PLACEHOLDER1, // beat
      PLACEHOLDER2, // reactive p1
      PLACEHOLDER3, // audio spectrum
      PLACEHOLDER4, // reactive p2
      HID,
      DISABLE,
      COUNT
    };

    void init();
    void set_rgb(rgb_light lights);
    void set_leds_off();
    void update(rgb_light lights,
                Mode mode);
  }
}

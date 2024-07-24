#pragma once

#include <FastLED/src/FastLED.h>

#include "../bpm.h"
#include "../config.h"

namespace IIDX {
  struct hid_lights {
    uint16_t buttons;
    rgb_light tt_lights;
    rgb_light bar_lights;
  } ATTR_PACKED;

  namespace RgbManager {
    void update(const config &config,
                const int8_t tt_report,
                const hid_lights &led_state_from_hid_report);
    namespace Turntable {
      void set_leds_off();
      void reverse_tt(uint8_t reverse_tt);
      void display_tt_change(const CRGB &colour,
                             uint8_t value,
                             uint8_t range);
    }

    namespace Bar {
      void set_leds_off();
    }
  }
}

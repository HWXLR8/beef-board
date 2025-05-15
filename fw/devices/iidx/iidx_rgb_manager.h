#pragma once

#include <FastLED/src/FastLED.h>

#include "../rgb_helper.h"

namespace IIDX {
  struct hid_lights {
    uint16_t buttons;
    rgb_light tt_lights;
    rgb_light bar_lights;
  } ATTR_PACKED;

  namespace RgbManager {
    void update(int8_t tt_report, const hid_lights &led_state_from_hid_report);
    namespace Turntable {
      extern bool force_update;

      bool set_leds_off();
      void reverse_tt(bool reverse_tt);
      void display_tt_change(const CRGB &colour,
                             uint8_t value,
                             uint8_t range);
    }

    namespace Bar {
      extern bool force_update;

      bool set_leds_off();
    }
  }
}

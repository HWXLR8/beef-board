#pragma once

#include <FastLED/src/FastLED.h>

#include "../bpm.h"
#include "../config.h"
#include "iidx_rgb.h"

namespace IIDX {
  namespace RgbManager {
    void update(const config &config,
                const int8_t tt_report,
                const hid_lights &led_state_from_hid_report);
    namespace Turntable {
      extern timer combo_timer;

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

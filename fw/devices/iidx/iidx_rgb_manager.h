#pragma once

#include <FastLED/src/FastLED.h>

#include "../bpm.h"
#include "../config.h"

namespace Iidx {
  namespace RgbManager {
    namespace Turntable {
      extern timer combo_timer;

      void set_leds_off();
      void reverse_tt(uint8_t reverse_tt);
      void display_tt_change(const CRGB &colour,
                             uint8_t value,
                             uint8_t range);
      void update(int8_t tt_report,
                  const rgb_light &lights,
                  const config &current_config);
    }

    namespace Bar {
      void set_leds_off();
      void update(const rgb_light &lights,
                  const config &current_config,
                  Bpm &bpm);
    }
  }
}

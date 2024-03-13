#pragma once

#include <FastLED/src/FastLED.h>

#include "bpm.h"
#include "config.h"
#include "rgb.h"
#include "timer.h"

#define RING_LIGHT_LEDS 24
#define LIGHT_BAR_LEDS 16

constexpr auto DEFAULT_COLOUR = HSV{ 128, 255, 255 }; // Aqua

namespace RgbManager {
  void init();

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
                const Bpm &bpm);
  }
}

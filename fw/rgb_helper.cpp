#include "beef.h"
#include "rgb_helper.h"
#include "rgb_patterns.h"

CRGB tt_leds[RING_LIGHT_LEDS] = {0};
CRGB bar_leds[LIGHT_BAR_LEDS] = {0};

namespace RgbHelper {
  // Pin mapping can be found in FastLED/src/platforms/avr/fastpin_avr.h
  enum {
    BAR_DATA_PIN = 14, // C4
    TT_DATA_PIN  = 15  // C5
  };

  void init() {
    FastLED.addLeds<NEOPIXEL, TT_DATA_PIN>(tt_leds, RING_LIGHT_LEDS)
        .setDither(DISABLE_DITHER);
    FastLED.addLeds<NEOPIXEL, BAR_DATA_PIN>(bar_leds, LIGHT_BAR_LEDS)
        .setDither(DISABLE_DITHER);
  }

  void set_rgb(CRGB* leds, const uint8_t n, const rgb_light &lights) {
    fill_solid(leds, n,
               CRGB(lights.r, lights.g, lights.b));
  }

  void set_hsv(CRGB* leds, const uint8_t n, const HSV &hsv) {
    CRGB rgb{};
    hsv2rgb_spectrum(CHSV(hsv.h, hsv.s, hsv.v), rgb);
    fill_solid(leds, n, rgb);
  }

  void breathing(BreathingPattern &breathing_pattern,
                 CRGB* leds, const uint8_t n, const HSV &hsv) {
    const auto v = breathing_pattern.update();
    set_hsv(leds, n, { hsv.h, hsv.s, v });
  }

  void hid(CRGB* leds, const uint8_t n, rgb_light lights) {
    // Share same lighting state between different lights for HID standby animation
    static BreathingPattern hid_standby = BreathingPattern();

    if (rgb_standby)
      breathing(hid_standby, leds, n, {});
    else
      set_rgb(leds, n, lights);
  }

  // update lighting on a timer to reduce the number of
  // computationally expensive calls to FastLED.show()
  // basically what FastLED does but without spin waiting
  bool ready_to_present() {
    static uint32_t last_show = 0;
    const uint32_t min_micros = 1000000 / BEEF_LED_REFRESH;
    const uint32_t now = micros();
    if (now-last_show < min_micros)
      return false;

    last_show = now;
    return true;
  }
}

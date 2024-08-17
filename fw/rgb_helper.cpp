#include "hid.h"
#include "rgb_helper.h"

CRGB tt_leds[RING_LIGHT_LEDS] = {0};
CRGB bar_leds[LIGHT_BAR_LEDS] = {0};

namespace RgbHelper {
  // Pin mapping can be found in FastLED/src/platforms/avr/fastpin_avr.h
  enum {
    BAR_DATA_PIN = 14, // C4
    TT_DATA_PIN  = 15  // C5
  };

  timer combo_timer{};

  CLEDController* tt_controller;
  CLEDController* bar_controller;

  void init() {
    timer_init(&combo_timer);

    tt_controller = &FastLED.addLeds<NEOPIXEL, TT_DATA_PIN>(tt_leds, RING_LIGHT_LEDS)
      .setDither(DISABLE_DITHER);
    bar_controller = &FastLED.addLeds<NEOPIXEL, BAR_DATA_PIN>(bar_leds, LIGHT_BAR_LEDS)
      .setDither(DISABLE_DITHER);
    FastLED.setMaxRefreshRate(0); // We have our own frame rate limiter
  }

  bool set_rgb(CRGB* leds, const uint8_t n, const rgb_light &lights) {
    return set_rgb(leds, n,
                   CRGB(lights.r, lights.g, lights.b));
  }

  bool set_rgb(CRGB* leds, const uint8_t n, const CRGB &rgb) {
    bool update = false;
    for (uint8_t i = 0; i < n; i++) {
      update |= leds[i] != rgb;
      leds[i] = rgb;
    }
    return update;
  }

  bool set_hsv(CRGB* leds, const uint8_t n, const HSV &hsv) {
    CRGB rgb{};
    hsv2rgb_spectrum(CHSV(hsv.h, hsv.s, hsv.v), rgb);
    return set_rgb(leds, n, rgb);
  }

  bool breathing(BreathingPattern &breathing_pattern,
                 CRGB* leds, const uint8_t n, const HSV &hsv) {
    const auto v = breathing_pattern.update();
    return set_hsv(leds, n, { hsv.h, hsv.s, v });
  }

  bool hid(CRGB* leds, const uint8_t n, rgb_light lights) {
    if (rgb_standby) {
      // Share same lighting state between different lights for HID standby animation
      static BreathingPattern hid_standby;
      return breathing(hid_standby, leds, n, {});
    }
    return set_rgb(leds, n, lights);
  }

  // update lighting on a timer to reduce the number of
  // computationally expensive calls to FastLED.show()
  // basically what FastLED does but without spin waiting
  bool ready_to_present(bool disable_leds) {
    if (disable_leds)
      return false;

    static uint32_t last_show = 0;
    const uint32_t min_micros = 1000000 / BEEF_LED_REFRESH;
    const uint32_t now = micros();
    if (now-last_show < min_micros)
      return false;

    last_show = now;
    return true;
  }

  void show_tt() {
    tt_controller->showLeds();
  }

  void show_bar() {
    bar_controller->showLeds();
  }
}

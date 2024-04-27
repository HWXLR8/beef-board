#include "beef.h"
#include "rgb_manager.h"
#include "rgb_patterns.h"
#include "ticker.h"

// Pin mapping can be found in FastLED/src/platforms/avr/fastpin_avr.h
#define BAR_DATA_PIN 14 // C4
#define TT_DATA_PIN  15 // C5

#define SPIN_TIMER 50
#define FAST_SPIN_TIMER 25
#define REACT_TIMER 500
#define BREATHING_DURATION 2048
#define BREATHING_TIMER 3000

namespace RgbManager {
  void set_rgb(CRGB* leds, uint8_t n, rgb_light lights) {
    fill_solid(leds, n,
               CRGB(lights.r, lights.g, lights.b));
  }

  void set_hsv(CRGB* leds, uint8_t n, HSV hsv) {
    CRGB rgb{};
    hsv2rgb_spectrum(CHSV(hsv.h, hsv.s, hsv.v), rgb);
    fill_solid(leds, n, rgb);
  }

  void breathing(BreathingPattern &breathing_pattern,
                 CRGB* leds, const int n, const HSV hsv) {
    const auto v = breathing_pattern.update();
    set_hsv(leds, n, { hsv.h, hsv.s, v });
  }

  void hid(CRGB* leds, int n,
           rgb_light lights) {
    // Share same lighting state between TT and light bar for HID standby animation
    static BreathingPattern hid_standby = BreathingPattern();

    if (rgb_standby)
      breathing(hid_standby, leds, n, {});
    else
      set_rgb(leds, n, lights);
  }

  namespace Turntable {
    // Add a second-long rest period
    BreathingPattern breathing_pattern(BREATHING_DURATION,
                                       BREATHING_TIMER);
    timer combo_timer;
    CRGB leds[RING_LIGHT_LEDS] = {0};

    void init() {
      timer_init(&combo_timer);

      FastLED.addLeds<NEOPIXEL, TT_DATA_PIN>(leds, RING_LIGHT_LEDS)
        .setDither(DISABLE_DITHER);
    }

    void set_hsv(HSV hsv) {
      RgbManager::set_hsv(leds, RING_LIGHT_LEDS, hsv);
    }

    void set_leds_off() {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Black);
    }

    // Render two spinning LEDs
    void spin(const HSV &hsv, int8_t tt_report) {
      static bool first_call = true;
      static uint8_t last_spin_counter = 0;
      static SpinPattern spin_pattern(SPIN_TIMER,
                                      FAST_SPIN_TIMER,
                                      RING_LIGHT_LEDS / 2);

      auto spin_counter = spin_pattern.update(tt_report);
      if (spin_counter != last_spin_counter || first_call) {
        set_leds_off();

        const auto colour = CHSV(hsv.h, 255, 255);
        leds[spin_counter] = colour;
        leds[spin_counter+12] = colour;
        first_call = false;
        last_spin_counter = spin_counter;
      }
    }

    void colour_shift(const HSV &hsv) {
      static uint8_t h;
      static Ticker ticker(100);

      h += ticker.get_ticks();
      set_hsv({ h, hsv.s, hsv.v });
    }

    void render_rainbow(const HSV &hsv, const uint8_t pos = 0) {
      fill_rainbow_circular(leds, RING_LIGHT_LEDS, pos);

      // Emulate HSV adjustments
      for (auto &led : leds) {
        led += (CRGB::White - led).
          scale8(255 - hsv.s);
        led.nscale8(hsv.v);
      }
    }

    void render_rainbow_react(const HSV &hsv, const int8_t tt_report) {
      static bool first_call = true;
      static uint8_t pos = 0;
      static Ticker t(3);

      const auto ticks = t.get_ticks();
      if (ticks > 0 || first_call) {
        pos += ticks * -tt_report * BEEF_TT_RAINBOW_SPIN_SPEED;
        render_rainbow(hsv, pos);
        first_call = false;
      }
    }

    void render_rainbow_spin(const HSV &hsv, const int8_t tt_report) {
      static SpinPattern spin_pattern(3, 2);

      auto pos = spin_pattern.update(tt_report) * BEEF_TT_RAINBOW_SPIN_SPEED;
      render_rainbow(hsv, -pos);
    }

    void react(const int8_t tt_report, const HSV &hsv) {
      static uint8_t h = hsv.h;
      static timer scr_timer;

      switch (tt_report) {
        case 1:
          h = hsv.h;
          timer_arm(&scr_timer, REACT_TIMER);
          break;
        case -1:
          h = hsv.h + 128;
          timer_arm(&scr_timer, REACT_TIMER);
          break;
        default:
          break;
      }

      const uint8_t v = timer_is_active(&scr_timer) ? 255 : 64;

      set_hsv({ h, hsv.s, v });
    }

    // Illuminate + as blue, - as red in two halves
    void reverse_tt(uint8_t reverse_tt) {
      int offset = reverse_tt ? 0 : RING_LIGHT_LEDS / 2;
      int blue_start = offset;
      int blue_end = blue_start + (RING_LIGHT_LEDS / 2);
      int red_start = (12 + offset) % RING_LIGHT_LEDS;
      int red_end = red_start + (RING_LIGHT_LEDS / 2);
      for (int i = blue_start; i < blue_end; ++i) {
        leds[i] = CRGB::Blue;
      }
      for (int i = red_start; i < red_end; ++i) {
        leds[i] = CRGB::Red;
      }
    }

    void display_tt_change(const CRGB &colour,
                           const uint8_t value,
                           const uint8_t range) {
      const uint8_t num_of_leds = value * (RING_LIGHT_LEDS / range);
      uint8_t i = 0;
      for (; i < num_of_leds; ++i) {
        leds[i] = colour;
      }
      for (; i < RING_LIGHT_LEDS; ++i) {
        leds[i] = CRGB::Black;
      }
      timer_arm(&combo_timer,
                CONFIG_CHANGE_NOTIFY_TIME);
    }

    // Match tt_report with physical turntable movement
    // -1 is clockwise, +1 is counter-clockwise
    int8_t normalise_tt_report(const bool reverse_tt,
                               const int8_t tt_report) {
      return reverse_tt ? tt_report : -tt_report;
    }

    void update(int8_t tt_report,
                const rgb_light &lights,
                const config &current_config) {
      // Ignore turntable effect if notifying a mode change
      if (!timer_is_active(&combo_timer)) {
        switch(current_config.tt_effect) {
          case TurntableMode::STATIC:
            set_hsv(current_config.tt_static_hsv);
            break;
          case TurntableMode::SPIN:
            tt_report = normalise_tt_report(current_config.reverse_tt,
                                            tt_report);
            spin(current_config.tt_spin_hsv, tt_report);
            break;
          case TurntableMode::SHIFT:
            colour_shift(current_config.tt_shift_hsv);
            break;
          case TurntableMode::RAINBOW_STATIC:
            render_rainbow(current_config.tt_rainbow_static_hsv);
            break;
          case TurntableMode::RAINBOW_REACT:
            tt_report = normalise_tt_report(current_config.reverse_tt,
                                            tt_report);
            render_rainbow_react(current_config.tt_rainbow_react_hsv,
                                 tt_report);
            break;
          case TurntableMode::RAINBOW_SPIN:
            tt_report = normalise_tt_report(current_config.reverse_tt,
                                            tt_report);
            render_rainbow_spin(current_config.tt_rainbow_spin_hsv,
                                tt_report);
            break;
          case TurntableMode::REACT:
            react(tt_report, current_config.tt_react_hsv);
            break;
          case TurntableMode::BREATHING:
            breathing(breathing_pattern,
                      leds, RING_LIGHT_LEDS,
                      current_config.tt_breathing_hsv);
            break;
          case TurntableMode::HID:
            hid(leds, RING_LIGHT_LEDS, lights);
            break;
          case TurntableMode::DISABLE:
            set_leds_off();
            break;
          default:
            break;
        }
      }
    }
  }

  namespace Bar {
    CRGB leds[LIGHT_BAR_LEDS] = {0};

    enum class SpectrumSide : uint8_t {
      P1,
      P2
    };

    void init() {
      FastLED.addLeds<NEOPIXEL, BAR_DATA_PIN>(leds, LIGHT_BAR_LEDS)
        .setDither(DISABLE_DITHER);
    }

    void set_leds_off() {
      fill_solid(leds, LIGHT_BAR_LEDS, CRGB::Black);
    }

    void spectrum(const uint8_t level, const SpectrumSide side) {
      static uint8_t last_level;

      if (last_level != level) {
        set_leds_off();
        fill_rainbow(leds, level, 0, -16);
        if (side == SpectrumSide::P1) {
          for (uint8_t i = 0; i < LIGHT_BAR_LEDS / 2; i++) {
            const auto tmp = leds[i];
            leds[i] = leds[LIGHT_BAR_LEDS-1-i];
            leds[LIGHT_BAR_LEDS-1-i] = tmp;
          }
        }
      }

      last_level = level;
    }

    void update(const rgb_light &lights,
                const config &current_config,
                const Bpm &bpm) {
      switch(current_config.bar_effect) {
        case BarMode::KEY_SPECTRUM_P1:
          spectrum(bpm.get(), SpectrumSide::P1);
          break;
        case BarMode::KEY_SPECTRUM_P2:
          spectrum(bpm.get(), SpectrumSide::P2);
          break;
        case BarMode::HID:
          hid(leds, LIGHT_BAR_LEDS, lights);
          break;
        case BarMode::DISABLE:
          set_leds_off();
          break;
        default:
          break;
      }
    }
  }

  void init() {
    Turntable::init();
    Bar::init();
  }
}

#include "../beef.h"
#include "../rgb_helper.h"
#include "iidx_rgb_manager.h"

#define SPIN_TIMER 50
#define FAST_SPIN_TIMER 25
#define REACT_TIMER 500
#define BREATHING_DURATION 2048
#define BREATHING_TIMER 3000

namespace Iidx {
  namespace RgbManager {
    namespace Turntable {
      // Add a second-long rest period
      BreathingPattern breathing_pattern(BREATHING_DURATION,
                                         BREATHING_TIMER);
      timer combo_timer{};

      void init() {
      }

      void set_hsv(HSV hsv) {
        RgbHelper::set_hsv(tt_leds, RING_LIGHT_LEDS, hsv);
      }

      void set_leds_off() {
        fill_solid(tt_leds, RING_LIGHT_LEDS, CRGB::Black);
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
          tt_leds[spin_counter] = colour;
          tt_leds[spin_counter+12] = colour;
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
        fill_rainbow_circular(tt_leds, RING_LIGHT_LEDS, pos);

        // Emulate HSV adjustments
        for (auto &led : tt_leds) {
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
          tt_leds[i] = CRGB::Blue;
        }
        for (int i = red_start; i < red_end; ++i) {
          tt_leds[i] = CRGB::Red;
        }

        timer_arm(&combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
      }

      void display_tt_change(const CRGB &colour,
                             const uint8_t value,
                             const uint8_t range) {
        const uint8_t num_of_leds = value * (RING_LIGHT_LEDS / range);
        uint8_t i = 0;
        for (; i < num_of_leds; ++i) {
          tt_leds[i] = colour;
        }
        for (; i < RING_LIGHT_LEDS; ++i) {
          tt_leds[i] = CRGB::Black;
        }

        timer_arm(&combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
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
        // Ignore turntable effect if notifying a setting change
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
              RgbHelper::breathing(breathing_pattern,
                                   tt_leds, RING_LIGHT_LEDS,
                                     current_config.tt_breathing_hsv);
              break;
            case TurntableMode::HID:
              RgbHelper::hid(tt_leds, RING_LIGHT_LEDS, lights);
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

      enum class SpectrumSide : uint8_t {
        P1,
        P2
      };

      void set_leds_off() {
        fill_solid(bar_leds, LIGHT_BAR_LEDS, CRGB::Black);
      }

      void spectrum(Bpm &bpm, const SpectrumSide side) {
        static uint8_t last_level;

        const auto level = bpm.update(button_state);

        if (last_level != level) {
          set_leds_off();
          fill_rainbow(bar_leds, level, 0, -16);
          if (side == SpectrumSide::P1) {
            for (uint8_t i = 0; i < LIGHT_BAR_LEDS / 2; i++) {
              const auto tmp = bar_leds[i];
              bar_leds[i] = bar_leds[LIGHT_BAR_LEDS-1-i];
              bar_leds[LIGHT_BAR_LEDS-1-i] = tmp;
            }
          }
        }

        last_level = level;
      }

      void update(const rgb_light &lights,
                  const config &current_config,
                  Bpm &bpm) {
        switch(current_config.bar_effect) {
          case BarMode::KEY_SPECTRUM_P1:
            spectrum(bpm, SpectrumSide::P1);
            break;
          case BarMode::KEY_SPECTRUM_P2:
            spectrum(bpm, SpectrumSide::P2);
            break;
          case BarMode::HID:
            RgbHelper::hid(bar_leds, LIGHT_BAR_LEDS, lights);
            break;
          case BarMode::DISABLE:
            set_leds_off();
            break;
          default:
            break;
        }
      }
    }
  }
}

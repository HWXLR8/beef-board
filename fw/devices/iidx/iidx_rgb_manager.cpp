#include "../beef.h"
#include "iidx_rgb_manager.h"

#define SPIN_TIMER 50 * RING_ANIM_NORMALISE
#define FAST_SPIN_TIMER 25 * RING_ANIM_NORMALISE
enum {
  REACT_TIMER = 500,
  BREATHING_DURATION = 2048,
  BREATHING_TIMER = 3000
};

namespace IIDX {
  namespace RgbManager {
    namespace Turntable {
      bool force_update;

      // Add a second-long rest period
      BreathingPattern breathing_pattern(BREATHING_DURATION,
                                         BREATHING_TIMER);

      bool set_hsv(HSV hsv) {
        return RgbHelper::set_hsv(tt_leds, RING_LIGHT_LEDS, hsv);
      }

      bool set_leds_off() {
        return RgbHelper::set_rgb(tt_leds, RING_LIGHT_LEDS, CRGB::Black);
      }

      SpinPattern spin_pattern(SPIN_TIMER,
                                      FAST_SPIN_TIMER,
                                      RING_LIGHT_LEDS / 2);
      // Render two spinning LEDs
      bool spin(const HSV &hsv, int8_t tt_report) {
        if (spin_pattern.update(tt_report)) {
          set_leds_off();

          const auto colour = CHSV(hsv.h, 255, 255);
          const auto spin_counter = spin_pattern.get();
          tt_leds[spin_counter] = colour;
          tt_leds[spin_counter+(RING_LIGHT_LEDS/2)] = colour;

          return true;
        }
        return false;
      }

      Ticker colour_shift_ticker(100);
      bool colour_shift(const HSV &hsv) {
        static uint8_t h;

        const auto ticks = colour_shift_ticker.get_ticks();
        if (ticks > 0) {
          h += ticks;
          return set_hsv({ h, hsv.s, hsv.v });
        }
        return false;
      }

      bool render_rainbow(const HSV &hsv, const uint8_t pos) {
        static uint8_t prev_pos;
        bool update = false;
        if (force_update || prev_pos != pos) {
          fill_rainbow_circular(tt_leds,
                                RING_LIGHT_LEDS,
                                -pos);
          update = true;
        }
        prev_pos = pos;

        // Emulate HSV adjustments
        for (auto &led : tt_leds) {
          const auto prev_led = led;
          led += (CRGB::White - led).
            scale8(255 - hsv.s);
          led.nscale8(hsv.v);
          update |= prev_led != led;
        }

        return update;
      }

      Ticker rainbow_react_ticker(3);
      bool render_rainbow_react(const HSV &hsv,
                                const int8_t tt_report) {
        static uint8_t pos = 0;

        const auto ticks = rainbow_react_ticker.get_ticks();
        if (ticks > 0) {
          pos += ticks * tt_report * BEEF_TT_RAINBOW_SPIN_SPEED;
          render_rainbow(hsv, pos);
          return true;
        }
        return false;
      }

      SpinPattern rainbow_spin_pattern(3, 2);
      bool render_rainbow_spin(const HSV &hsv,
                               const int8_t tt_report) {
        if (rainbow_spin_pattern.update(tt_report)) {
          const uint8_t pos = rainbow_spin_pattern.get() * BEEF_TT_RAINBOW_SPIN_SPEED;
          return render_rainbow(hsv, pos);
        }
        return false;
      }

      bool react(const int8_t tt_report, const HSV &hsv) {
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

        return set_hsv({ h, hsv.s, v });
      }

      // Illuminate + as blue, - as red in two halves
      void reverse_tt(bool reverse_tt) {
        auto first_half = CRGB::Blue;
        auto second_half = CRGB::Red;
        if (reverse_tt) {
          first_half = CRGB::Red;
          second_half = CRGB::Blue;
        }

        uint8_t i = 0;
        for (; i < RING_LIGHT_LEDS / 2; i++) {
          tt_leds[i] = first_half;
        }
        for (; i < RING_LIGHT_LEDS; i++) {
          tt_leds[i] = second_half;
        }

        timer_arm(&RgbHelper::combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
        force_update = true;
      }

      void display_tt_change(const CRGB &colour,
                             const uint8_t value,
                             const uint8_t range) {
        const uint8_t num_of_leds = value * (RING_LIGHT_LEDS / range);
        uint8_t i = 0;
        for (; i < num_of_leds; i++) {
          tt_leds[i] = colour;
        }
        for (; i < RING_LIGHT_LEDS; i++) {
          tt_leds[i] = CRGB::Black;
        }

        timer_arm(&RgbHelper::combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
        force_update = true;
      }

      // Match tt_report with physical turntable movement
      // -1 is clockwise, +1 is counter-clockwise
      int8_t normalise_tt_report(const bool reverse_tt,
                                 const int8_t tt_report) {
        return reverse_tt ? tt_report : -tt_report;
      }

      bool update(int8_t tt_report,
                  const rgb_light &lights,
                  const config &current_config) {
        auto update = force_update;

        // Ignore turntable effect if notifying a setting change
        if (timer_is_active(&RgbHelper::combo_timer)) {
          force_update = false;
          return update;
        }

        switch(current_config.tt_effect) {
          case TurntableMode::Static:
            update |= set_hsv(current_config.tt_static_hsv);
            break;
          case TurntableMode::Spin:
            tt_report = normalise_tt_report(current_config.reverse_tt,
                                            tt_report);
            update |= spin(current_config.tt_spin_hsv, tt_report);
            break;
          case TurntableMode::Shift:
            update |= colour_shift(current_config.tt_shift_hsv);
            break;
          case TurntableMode::RainbowStatic:
            update |= render_rainbow(current_config.tt_rainbow_static_hsv, 0);
            break;
          case TurntableMode::RainbowReact:
            tt_report = normalise_tt_report(current_config.reverse_tt,
                                            tt_report);
            update |= render_rainbow_react(current_config.tt_rainbow_react_hsv,
                                           tt_report);
            break;
          case TurntableMode::RainbowSpin:
            tt_report = normalise_tt_report(current_config.reverse_tt,
                                            tt_report);
            update |= render_rainbow_spin(current_config.tt_rainbow_spin_hsv,
                                          tt_report);
            break;
          case TurntableMode::React:
            update |= react(tt_report, current_config.tt_react_hsv);
            break;
          case TurntableMode::Breathing:
            update |= RgbHelper::breathing(breathing_pattern,
                                           tt_leds, RING_LIGHT_LEDS,
                                           current_config.tt_breathing_hsv);
            break;
          case TurntableMode::HID:
            update |= RgbHelper::hid(tt_leds, RING_LIGHT_LEDS, lights);
            break;
          case TurntableMode::Disable:
            update |= set_leds_off();
            break;
          default:
            break;
        }

        force_update = false;
        return update;
      }
    }

    namespace Bar {
      bool force_update;

      enum class SpectrumSide : uint8_t {
        P1,
        P2
      };

      bool set_leds_off() {
        return RgbHelper::set_rgb(bar_leds, LIGHT_BAR_LEDS, CRGB::Black);
      }

      Bpm bpm(LIGHT_BAR_LEDS);
      bool spectrum(const SpectrumSide side) {
        static uint8_t last_level;

        const auto level = bpm.update(button_state);

        bool update = false;
        if (last_level != level) {
          update = true;
          set_leds_off();
          fill_rainbow(bar_leds, level, 0, -16);
          if (side == SpectrumSide::P1) {
            // Flip for P1
            for (uint8_t i = 0; i < LIGHT_BAR_LEDS / 2; i++) {
              const auto tmp = bar_leds[i];
              bar_leds[i] = bar_leds[LIGHT_BAR_LEDS-1-i];
              bar_leds[LIGHT_BAR_LEDS-1-i] = tmp;
            }
          }
        }

        last_level = level;
        return update;
      }

      bool update(const rgb_light &lights,
                  const config &current_config) {
        auto update = force_update;

        switch(current_config.bar_effect) {
          case BarMode::KeySpectrumP1:
            update |= spectrum(SpectrumSide::P1);
            break;
          case BarMode::KeySpectrumP2:
            update |= spectrum(SpectrumSide::P2);
            break;
          case BarMode::HID:
            update |= RgbHelper::hid(bar_leds, LIGHT_BAR_LEDS, lights);
            break;
          case BarMode::Disable:
            update |= set_leds_off();
            break;
          default:
            break;
        }

        force_update = false;
        return update;
      }
    }

    void update(const config &config,
                const int8_t tt_report,
                const hid_lights &led_state_from_hid_report) {
      if (!RgbHelper::ready_to_present(config.disable_led)) {
        return;
      }

#if RING_LIGHT_LEDS > 0
      if (Turntable::update(tt_report,
                            led_state_from_hid_report.tt_lights,
                            config)) {
        RgbHelper::show_tt();
      }
#endif

#if LIGHT_BAR_LEDS > 0
      if (Bar::update(led_state_from_hid_report.bar_lights,
                      config)) {
        RgbHelper::show_bar();
      }
#endif
    }
  }
}

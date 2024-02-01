#include "rgb_manager.h"

// Pin mapping can be found in FastLED/src/paltforms/avr/fastpin_avr.h
#define BAR_DATA_PIN 14 // C4
#define TT_DATA_PIN  15 // C5

#define SPIN_TIMER 50

namespace RgbManager {
  namespace Turntable {
    timer combo_timer;
    CRGB leds[RING_LIGHT_LEDS] = {0};
    timer scr_timer;

    void init() {
      static bool inited = false;
      if (!inited) {
        inited = true;

        timer_init(&scr_timer);
        timer_init(&combo_timer);

        FastLED.addLeds<NEOPIXEL, TT_DATA_PIN>(leds, RING_LIGHT_LEDS)
          .setDither(DISABLE_DITHER);
      } 
    }

    void set_leds(rgb_light lights) {
      fill_solid(leds, RING_LIGHT_LEDS,
                 CRGB(lights.r, lights.g, lights.b));
    }

    void set_leds_blue(void) {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Blue);
    }

    void set_leds_red(void) {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Red);
    }

    void set_leds_off(void) {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Black);
    }

    // Render two spinning turquoise LEDs
    void spin(void) {
      static uint8_t spin_counter = 0;
      EVERY_N_MILLIS(SPIN_TIMER) {
        spin_counter = (spin_counter + 1) % (RING_LIGHT_LEDS / 2);
        set_leds_off();
        leds[spin_counter] = CRGB::Turquoise;
        leds[spin_counter+12] = CRGB::Turquoise;
      }
    }

    void react_to_scr(int8_t tt_report) {
      if (tt_report == 1) {
        set_leds_blue();
        timer_arm(&scr_timer, 500);
      } else if (tt_report == -1) {
        set_leds_red();
        timer_arm(&scr_timer, 500);
      }
      if (!timer_is_active(&scr_timer)) {
        set_leds_off();
      }
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

    void display_tt_change(uint8_t deadzone, int range) {
      int num_of_leds = deadzone * (RING_LIGHT_LEDS / range);
      for (int i = 0; i < num_of_leds; ++i) {
        leds[i] = CRGB::Red;
      }
      for (int i = num_of_leds; i < RING_LIGHT_LEDS; ++i) {
        leds[i] = CRGB::Black;
      }
    }

    // tt +1 is counter-clockwise, -1 is clockwise
    void update(int8_t tt_report,
                rgb_light lights,
                Mode mode) {
      // Ignore turtable effect if notifying a mode change
      if (!timer_is_active(&combo_timer)) {
        switch(mode) {
          case SPIN:
            spin();
            break;
          case REACT_TO_SCR:
            react_to_scr(tt_report);
            break;
          case HID:
            set_leds(lights);
            break;
          case DISABLE:
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

    void init() {
      static bool inited = false;
      if (!inited) {
        inited = true;

        FastLED.addLeds<NEOPIXEL, BAR_DATA_PIN>(leds, LIGHT_BAR_LEDS)
          .setDither(DISABLE_DITHER);
      }
    }

    void set_leds(rgb_light lights) {
      fill_solid(leds, LIGHT_BAR_LEDS,
                 CRGB(lights.r, lights.g, lights.b));
    }

    void set_leds_off(void) {
      fill_solid(leds, LIGHT_BAR_LEDS, CRGB::Black);
    }

    void update(rgb_light lights,
                Mode mode) {
      switch(mode) {
        case HID:
          set_leds(lights);
          break;
        case DISABLE:
          set_leds_off();
          break;
        default:
          break;
      }
    }
  }
}

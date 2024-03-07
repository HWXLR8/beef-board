#include "beef.h"
#include "rgb_manager.h"
#include "ticker.h"

// Pin mapping can be found in FastLED/src/platforms/avr/fastpin_avr.h
#define BAR_DATA_PIN 14 // C4
#define TT_DATA_PIN  15 // C5

#define SPIN_TIMER 50
#define BREATHING_TIMER 3000

namespace RgbManager {
  void set_rgb(CRGB* leds, uint8_t n, rgb_light lights) {
    fill_solid(leds, n,
               CRGB(lights.r, lights.g, lights.b));
  }

  void set_hsv(CRGB* leds, uint8_t n, HSV hsv) {
    CRGB rgb{};
    hsv2rgb_spectrum(CHSV{hsv.h, hsv.s, hsv.v}, rgb);
    fill_solid(leds, n, rgb);
  }

  // Cycle from zero to full-bright to zero in around 2 seconds
  // Share same lighting state between TT and light bar
  template<int DURATION, int TIMER>
  void breathing(CRGB* leds, int n,
                 uint8_t h, uint8_t s) {
    static uint8_t theta = 0;
    static Ticker sin_ticker(8, DURATION);
    static timer breathing_timer{};
    static uint8_t v = 0;

    if (!timer_is_active(&breathing_timer)) {
      sin_ticker.reset();
      timer_arm(&breathing_timer, TIMER);
    }

    auto ticks = sin_ticker.get_ticks();
    if (ticks > 0) {
      theta += ticks;
      v = quadwave8(theta);
    }

    set_hsv(leds, n, HSV{h, s, v});
  }

  void hid(CRGB* leds, int n,
           rgb_light lights) {
    if (rgb_standby)
      breathing<0, 0>(leds, n, 0, 0);
    else
      set_rgb(leds, n, lights);
  }

  namespace Turntable {
    timer combo_timer;
    CRGB leds[RING_LIGHT_LEDS] = {0};
    timer scr_timer;

    void init() {
      static bool inited = false;
      if (!inited) {
        inited = true;

        timer_init(&combo_timer);
        timer_init(&scr_timer);

        FastLED.addLeds<NEOPIXEL, TT_DATA_PIN>(leds, RING_LIGHT_LEDS)
          .setDither(DISABLE_DITHER);
      } 
    }

    void set_rgb(rgb_light lights) {
      RgbManager::set_rgb(leds, RING_LIGHT_LEDS, lights);
    }

    void set_hsv(HSV hsv) {
      RgbManager::set_hsv(leds, RING_LIGHT_LEDS, hsv);
    }

    void set_leds_blue() {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Blue);
    }

    void set_leds_red() {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Red);
    }

    void set_leds_off() {
      fill_solid(leds, RING_LIGHT_LEDS, CRGB::Black);
    }

    void colour_shift() {
      static HSV hsv = { 0, 255, 255 };
      static Ticker ticker(100);

      hsv.h += ticker.get_ticks();
      set_hsv(hsv);
    }

    // Render two spinning turquoise LEDs
    void spin() {
      static bool first_call = true;
      static uint8_t spin_counter = 0;
      static Ticker ticker(SPIN_TIMER);

      auto ticks = ticker.get_ticks();
      if (ticks > 0 || first_call) {
        spin_counter = (spin_counter + ticks) % (RING_LIGHT_LEDS / 2);
        set_leds_off();
        leds[spin_counter] = CRGB::Aqua;
        leds[spin_counter+12] = CRGB::Aqua;
        first_call = false;
      }
    }

    void render_rainbow(uint8_t pos = 0) {
      fill_rainbow_circular(leds, RING_LIGHT_LEDS, pos);
    }

    void render_rainbow_pos(int8_t tt_report) {
      static uint8_t pos = 0;
      pos += -tt_report * BEEF_TT_RAINBOW_SPIN_SPEED;
      render_rainbow(pos);
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
                HSV hsv,
                Mode mode) {
      // Ignore turtable effect if notifying a mode change
      if (!timer_is_active(&combo_timer)) {
        switch(mode) {
          case Mode::STATIC:
            set_hsv(hsv);
            break;
          case Mode::SHIFT:
            colour_shift();
            break;
          case Mode::SPIN:
            spin();
            break;
          case Mode::RAINBOW_STATIC:
            render_rainbow();
            break;
          case Mode::RAINBOW_REACT:
            render_rainbow_pos(tt_report);
            break;
          case Mode::RAINBOW_SPIN:
            render_rainbow_pos(1);
            break;
          case Mode::REACT_TO_SCR:
            react_to_scr(tt_report);
            break;
          case Mode::BREATHING:
            // Add a second-long rest period
            breathing<2048, BREATHING_TIMER>(leds, RING_LIGHT_LEDS,
              default_colour.h, default_colour.s);
            break;
          case Mode::HID:
            hid(leds, RING_LIGHT_LEDS, lights);
            break;
          case Mode::DISABLE:
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

    void set_rgb(rgb_light lights) {
      RgbManager::set_rgb(leds, LIGHT_BAR_LEDS, lights);
    }

    void set_leds_off() {
      fill_solid(leds, LIGHT_BAR_LEDS, CRGB::Black);
    }

    void update(rgb_light lights,
                Mode mode) {
      switch(mode) {
        case Mode::HID:
          hid(leds, LIGHT_BAR_LEDS, lights);
          break;
        case Mode::DISABLE:
          set_leds_off();
          break;
        default:
          break;
      }
    }
  }
}

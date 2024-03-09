#pragma once

#include "pin.h"
#include "rgb.h"

#define CONFIG_ALL_HW_PIN { \
  CONFIG_HW_PIN(E0, E1),  /* BUTTON 1  */ \
  CONFIG_HW_PIN(B6, B7),  /* BUTTON 2  */ \
  CONFIG_HW_PIN(D4, D5),  /* BUTTON 3  */ \
  CONFIG_HW_PIN(D2, D3),  /* BUTTON 4  */ \
  CONFIG_HW_PIN(D0, D1),  /* BUTTON 5  */ \
  CONFIG_HW_PIN(D6, D7),  /* BUTTON 6  */ \
  CONFIG_HW_PIN(B4, B5),  /* BUTTON 7  */ \
  CONFIG_HW_PIN(C0, C1),  /* BUTTON 8  */ \
  CONFIG_HW_PIN(A6, A5),  /* BUTTON 9  */ \
  CONFIG_HW_PIN(E2, A7),  /* BUTTON 10 */ \
  CONFIG_HW_PIN(C2, C3),  /* BUTTON 11 */ \
}

// you still need to assign DDR and PORT manually in main() for now
// DDRF  &= 0b11111100;
// PORTF |= 0b00000011;

#define BUTTONS 11

#define CONFIG_CHANGE_NOTIFY_TIME 1000

#if BEEF_LED_REFRESH <= 0
  #define BEEF_LED_REFRESH 1
#elif BEEF_LED_REFRESH > 400
  #define BEEF_LED_REFRESH 400 // FastLED has a default max refresh rate of 400 for WS2812 LEDs
#endif

static_assert(BEEF_TT_RATIO != 0, "BEEF_TT_RATIO must not be 0");

// Do not reorder these fields
struct config {
  uint8_t version;
  uint8_t reverse_tt;
  TurntableMode tt_effect;
  uint8_t tt_deadzone;
  BarMode bar_effect;
  uint8_t disable_led;
  HSV tt_static_hsv;
  HSV tt_spin_hsv;
  HSV tt_shift_hsv;
  HSV tt_rainbow_static_hsv;
  HSV tt_rainbow_react_hsv;
  HSV tt_rainbow_spin_hsv;
  HSV tt_react_hsv;
  HSV tt_breathing_hsv;
};

struct callback {
  uint8_t* addr;
  uint8_t val;
};

void config_init(config* self);
void config_update(uint8_t* addr, uint8_t val);
callback toggle_reverse_tt(config* self);
callback cycle_tt_effects(config* self);
callback tt_hsv_set_hue(config* self);
callback tt_hsv_set_sat(config* self);
callback tt_hsv_set_val(config* self);
callback increase_deadzone(config* self);
callback decrease_deadzone(config* self);
callback cycle_bar_effects(config* self);
callback toggle_disable_led(config* self);

// button combos
#define REVERSE_TT_COMBO (BUTTON_1 | BUTTON_7 | BUTTON_8)
#define TT_EFFECTS_COMBO (BUTTON_2 | BUTTON_8 | BUTTON_11)
#define TT_DEADZONE_INCR_COMBO (BUTTON_3 | BUTTON_8 | BUTTON_11)
#define TT_DEADZONE_DECR_COMBO (BUTTON_1 | BUTTON_8 | BUTTON_11)
#define BAR_EFFECTS_COMBO (BUTTON_6 | BUTTON_8 | BUTTON_10)
#define DISABLE_LED_COMBO (BUTTON_4 | BUTTON_8 | BUTTON_11)
#define TT_HSV_HUE_COMBO (BUTTON_2 | BUTTON_11)
#define TT_HSV_SAT_COMBO (BUTTON_4 | BUTTON_11)
#define TT_HSV_VAL_COMBO (BUTTON_6 | BUTTON_11)

#pragma once

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
  CONFIG_HW_PIN(A4, A3),  /* BUTTON 9  */ \
  CONFIG_HW_PIN(A6, A5),  /* BUTTON 10 */ \
  CONFIG_HW_PIN(C2, C3),  /* BUTTON 11 */ \
}

enum {
  BUTTONS = 11,

  CONFIG_CHANGE_NOTIFY_TIME = 1000
};

#ifndef FW_VER
#define FW_VER 0xDEADBEEF
#endif

enum class ControllerType : uint8_t {
  IIDX,
  SDVX
};

enum class InputMode : uint8_t {
  Joystick,
  Keyboard
};

enum class Command : uint8_t {
  None,
  Bootloader,
  ResetConfig
};

// Key mapping structures
struct IIDXKeyMapping {
  uint8_t key_codes[13];
  uint8_t padding[7];
};

struct SDVXKeyMapping {
  uint8_t key_codes[9];
  uint8_t padding[11];
};

// Do not reorder these fields
struct config {
  uint8_t version;
  uint8_t reverse_tt;
  TurntableMode tt_effect;
  uint8_t tt_deadzone;
  BarMode bar_effect;
  uint8_t disable_leds;
  HSV tt_static_hsv;
  HSV tt_spin_hsv;
  HSV tt_shift_hsv;
  HSV tt_rainbow_static_hsv;
  HSV tt_rainbow_react_hsv;
  HSV tt_rainbow_spin_hsv;
  HSV tt_react_hsv;
  HSV tt_breathing_hsv;
  uint8_t tt_ratio;
  ControllerType controller_type;
  InputMode iidx_input_mode;
  InputMode sdvx_input_mode;
  uint8_t tt_sustain_ms;
  IIDXKeyMapping iidx_keys;
  SDVXKeyMapping sdvx_keys;
  uint8_t iidx_buttons_debounce;
  uint8_t iidx_effectors_debounce;
  uint8_t sdvx_buttons_debounce;
  uint8_t led_refresh;
  uint8_t rainbow_spin_speed;
  uint8_t tt_leds;
};

struct callback {
  uint8_t* addr;
  uint8_t val;
};

extern config current_config;

void config_init(config* self);
void config_update(config* self);
void config_update_setting(uint8_t* addr, uint8_t val);
void config_save(const config &new_config);
void set_controller_type(config &self, ControllerType mode);
void set_input_mode(config &self, InputMode mode);

callback toggle_reverse_tt(config* self);
callback cycle_tt_effects(config* self);
callback tt_hsv_set_hue(config* self);
callback tt_hsv_set_sat(config* self);
callback tt_hsv_set_val(config* self);
callback increase_deadzone(config* self);
callback decrease_deadzone(config* self);
callback increase_ratio(config* self);
callback decrease_ratio(config* self);
callback cycle_bar_effects(config* self);
callback toggle_disable_leds(config* self);

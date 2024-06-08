#define CONFIG_BASE_ADDR (uint8_t*)2
#define CONFIG_VERSION_ADDR (CONFIG_BASE_ADDR + offsetof(config, version))
#define CONFIG_REVERSE_TT_ADDR (CONFIG_BASE_ADDR + offsetof(config, reverse_tt))
#define CONFIG_TT_EFFECT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_effect))
#define CONFIG_TT_DEADZONE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_deadzone))
#define CONFIG_BAR_EFFECT_ADDR (CONFIG_BASE_ADDR + offsetof(config, bar_effect))
#define CONFIG_DISABLE_LED_ADDR (CONFIG_BASE_ADDR + offsetof(config, disable_led))
#define CONFIG_TT_STATIC_HUE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_static_hsv.h))
#define CONFIG_TT_STATIC_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_static_hsv.s))
#define CONFIG_TT_STATIC_VAL_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_static_hsv.v))
#define CONFIG_TT_SPIN_HUE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_spin_hsv.h))
#define CONFIG_TT_SHIFT_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_shift_hsv.s))
#define CONFIG_TT_SHIFT_VAL_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_shift_hsv.v))
#define CONFIG_TT_RAINBOW_STATIC_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_rainbow_static_hsv.s))
#define CONFIG_TT_RAINBOW_REACT_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_rainbow_react_hsv.s))
#define CONFIG_TT_RAINBOW_SPIN_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_rainbow_spin_hsv.s))
#define CONFIG_TT_REACT_HUE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_react_hsv.h))
#define CONFIG_TT_REACT_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_react_hsv.s))
#define CONFIG_TT_BREATHING_HUE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_breathing_hsv.h))
#define CONFIG_TT_BREATHING_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_breathing_hsv.s))
#define CONFIG_TT_RATIO_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_ratio))
#define CONFIG_USB_MODE (CONFIG_BASE_ADDR + offsetof(config, usb_mode))

enum {
  CONFIG_VERSION = 9,
  MAGIC = 0xBEEF
};

#include <avr/eeprom.h>

#include "devices/iidx/iidx_rgb_manager.h"

#include "analog_turntable.h"
#include "beef.h"
#include "config.h"
#include "rgb_helper.h"

// initialize config with default values
void init_config(config* self) {
  self->version = CONFIG_VERSION;
  self->reverse_tt = 0;
  self->tt_effect = TurntableMode::SPIN;
  self->tt_deadzone = 4;
  self->bar_effect = BarMode::HID;
  self->disable_led = 0;
  self->tt_static_hsv = DEFAULT_COLOUR;
  self->tt_spin_hsv = DEFAULT_COLOUR;
  self->tt_shift_hsv = { 0, 255, 255 };
  self->tt_rainbow_react_hsv = { 0, 255, 255 };
  self->tt_rainbow_react_hsv = { 0, 255, 255 };
  self->tt_rainbow_spin_hsv = { 0, 255, 255 };
  self->tt_react_hsv = DEFAULT_COLOUR;
  self->tt_breathing_hsv = DEFAULT_COLOUR;
  self->tt_ratio = 2;
}

bool update_config(config* self) {
  bool update = false;

  switch (self->version) {
    case 0:
      self->tt_effect = TurntableMode::SPIN;
      self->version++;
    case 1:
      self->tt_deadzone = 4;
      self->version++;
    case 2:
      self->bar_effect = BarMode::HID;
      self->version++;
    case 3:
      self->disable_led = 0;
      self->version++;
    case 4:
      self->tt_static_hsv = DEFAULT_COLOUR;
      self->version++;
    case 5:
      if (self->tt_effect >= TurntableMode::RAINBOW_SPIN) // Added new effect
        self->tt_effect = TurntableMode(uint8_t(self->tt_effect)+1);
      self->version++;
    case 6:
      self->tt_spin_hsv = DEFAULT_COLOUR;
      self->tt_shift_hsv = { 0, 255, 255 };
      self->tt_rainbow_react_hsv = { 0, 255, 255 };
      self->tt_rainbow_react_hsv = { 0, 255, 255 };
      self->tt_rainbow_spin_hsv = { 0, 255, 255 };
      self->tt_react_hsv = DEFAULT_COLOUR;
      self->tt_breathing_hsv = DEFAULT_COLOUR;
      self->version++;
    case 7:
      self->tt_ratio = 2;
      self->version++;
    case 8:
      self->usb_mode = UsbMode::IIDX;
      self->version++;
      update = true;
    default:
      break;
  }

  return update;
}

void config_init(config* self) {
  bool update;
  const auto magic = eeprom_read_word(nullptr);
  if (magic != MAGIC) {
    update = true;
    eeprom_write_word(nullptr, MAGIC);
    init_config(self);
  } else {
    eeprom_read_block(self, CONFIG_BASE_ADDR, sizeof(*self));
    update = update_config(self);
  }

  if (update)
    eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
}

void config_update(uint8_t* addr, const uint8_t val) {
  eeprom_update_byte(addr, val);
}

void set_mode(config &self, UsbMode mode) {
  self.usb_mode = mode;
  eeprom_update_byte(CONFIG_USB_MODE, static_cast<uint8_t>(self.usb_mode));
}

callback toggle_reverse_tt(config* self) {
  self->reverse_tt ^= 1;
  eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);

  update_tt_transitions(self->reverse_tt);
  Iidx::RgbManager::Turntable::reverse_tt(self->reverse_tt);

  return callback{};
}

callback cycle_tt_effects(config* self) {
  self->tt_effect = TurntableMode((uint8_t(self->tt_effect) + 1) % uint8_t(TurntableMode::COUNT));
  eeprom_write_byte(CONFIG_TT_EFFECT_ADDR, uint8_t(self->tt_effect));

  Iidx::RgbManager::Turntable::set_leds_off();

  return callback{};
}

callback tt_hsv_set_hue(config* self) {
  uint8_t* addr;
  HSV* h;

  switch (self->tt_effect) {
    case TurntableMode::STATIC:
      addr = CONFIG_TT_STATIC_HUE_ADDR;
      h = &self->tt_static_hsv;
      break;
    case TurntableMode::SPIN:
      addr = CONFIG_TT_SPIN_HUE_ADDR;
      h = &self->tt_spin_hsv;
      break;
    case TurntableMode::REACT:
      addr = CONFIG_TT_REACT_HUE_ADDR;
      h = &self->tt_react_hsv;
      break;
    case TurntableMode::BREATHING:
      addr = CONFIG_TT_BREATHING_HUE_ADDR;
      h = &self->tt_breathing_hsv;
      break;
    default:
      return callback{};
  }

  h->h += tt1.raw_state;

  return callback{addr, h->h};
}

callback tt_hsv_set_sat(config* self) {
  uint8_t* addr;
  HSV* s;

  switch (self->tt_effect) {
    case TurntableMode::STATIC:
      addr = CONFIG_TT_STATIC_SAT_ADDR;
      s = &self->tt_static_hsv;
      break;
    case TurntableMode::SHIFT:
      addr = CONFIG_TT_SHIFT_SAT_ADDR;
      s = &self->tt_shift_hsv;
      break;
    case TurntableMode::RAINBOW_STATIC:
      addr = CONFIG_TT_RAINBOW_STATIC_SAT_ADDR;
      s = &self->tt_rainbow_static_hsv;
      break;
    case TurntableMode::RAINBOW_REACT:
      addr = CONFIG_TT_RAINBOW_REACT_SAT_ADDR;
      s = &self->tt_rainbow_react_hsv;
      break;
    case TurntableMode::RAINBOW_SPIN:
      addr = CONFIG_TT_RAINBOW_SPIN_SAT_ADDR;
      s = &self->tt_rainbow_spin_hsv;
      break;
    case TurntableMode::REACT:
      addr = CONFIG_TT_REACT_SAT_ADDR;
      s = &self->tt_react_hsv;
      break;
    case TurntableMode::BREATHING:
      addr = CONFIG_TT_BREATHING_SAT_ADDR;
      s = &self->tt_breathing_hsv;
      break;
    default:
      return callback{};
  }

  const auto tt1_report = tt1.raw_state;

  if ((s->s == 0 && tt1_report < 0) ||
      (s->s == 255 && tt1_report > 0)) // prevent looping around
    return callback{};

  s->s += tt1_report;

  return callback{addr, s->s};
}

callback tt_hsv_set_val(config* self) {
  uint8_t* addr;
  HSV* v;

  switch (self->tt_effect) {
    case TurntableMode::STATIC:
      addr = CONFIG_TT_STATIC_VAL_ADDR;
      v = &self->tt_static_hsv;
      break;
    case TurntableMode::SHIFT:
      addr = CONFIG_TT_SHIFT_VAL_ADDR;
      v = &self->tt_shift_hsv;
      break;
    case TurntableMode::RAINBOW_STATIC:
      addr = CONFIG_TT_RAINBOW_STATIC_SAT_ADDR;
      v = &self->tt_rainbow_static_hsv;
      break;
    case TurntableMode::RAINBOW_REACT:
      addr = CONFIG_TT_RAINBOW_REACT_SAT_ADDR;
      v = &self->tt_rainbow_react_hsv;
      break;
    case TurntableMode::RAINBOW_SPIN:
      addr = CONFIG_TT_RAINBOW_SPIN_SAT_ADDR;
      v = &self->tt_rainbow_spin_hsv;
      break;
    default:
      return callback{};
  }

  const auto tt1_report = tt1.raw_state;

  if ((v->v == 0 && tt1_report < 0) ||
      (v->v == 255 && tt1_report > 0)) // prevent looping around
    return callback{};

  v->v += tt1_report;

  return callback{addr, v->v};
}

enum {
  DEADZONE_MAX = 6,
  DEADZONE_MIN = 1
};
void update_deadzone(const uint8_t deadzone) {
  eeprom_update_byte(CONFIG_TT_DEADZONE_ADDR, deadzone);

  tt1.deadzone = deadzone;

  Iidx::RgbManager::Turntable::display_tt_change(CRGB::Green,
                                                 deadzone,
                                                 DEADZONE_MAX);
}

callback increase_deadzone(config* self) {
  if (++self->tt_deadzone > DEADZONE_MAX)
    self->tt_deadzone = DEADZONE_MAX;

  update_deadzone(self->tt_deadzone);

  return callback{};
}

callback decrease_deadzone(config* self) {
  if (--self->tt_deadzone < DEADZONE_MIN)
    self->tt_deadzone = DEADZONE_MIN;

  update_deadzone(self->tt_deadzone);

  return callback{};
}

enum {
  RATIO_MAX = 6,
  RATIO_MIN = 1
};
void update_ratio(const uint8_t ratio) {
  eeprom_update_byte(CONFIG_TT_RATIO_ADDR, ratio);

  // Present TT ratio as TT sensitivity to the user
  Iidx::RgbManager::Turntable::display_tt_change(CRGB::Red,
                                                 RATIO_MAX+1-ratio,
                                                 RATIO_MAX);
}

callback increase_ratio(config* self) {
  if (++self->tt_ratio > RATIO_MAX)
    self->tt_ratio = RATIO_MAX;

  update_ratio(self->tt_ratio);

  return callback{};
}

callback decrease_ratio(config* self) {
  if (--self->tt_ratio < RATIO_MIN)
    self->tt_ratio = RATIO_MIN;

  update_ratio(self->tt_ratio);

  return callback{};
}

callback cycle_bar_effects(config* self) {
  do {
    self->bar_effect = BarMode((uint8_t(self->bar_effect) + 1) % uint8_t(BarMode::COUNT));
  } while (self->bar_effect == BarMode::PLACEHOLDER1 ||
           self->bar_effect == BarMode::PLACEHOLDER3);
  eeprom_write_byte(CONFIG_BAR_EFFECT_ADDR, uint8_t(self->bar_effect));

  Iidx::RgbManager::Bar::set_leds_off();

  return callback{};
}

callback toggle_disable_led(config* self) {
  self->disable_led ^= 1;

  eeprom_write_byte(CONFIG_DISABLE_LED_ADDR, self->disable_led);

  if (self->disable_led)
    FastLED.clear(true);

  return callback{};
}

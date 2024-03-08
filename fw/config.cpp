#pragma region Addresses
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
#define CONFIG_TT_REACT_HUE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_react_hsv.h))
#define CONFIG_TT_REACT_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_react_hsv.s))
#define CONFIG_TT_BREATHING_HUE_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_breathing_hsv.h))
#define CONFIG_TT_BREATHING_SAT_ADDR (CONFIG_BASE_ADDR + offsetof(config, tt_breathing_hsv.s))
#pragma endregion Addresses

#define CONFIG_VERSION 7
#define MAGIC 0xBEEF

#include <avr/eeprom.h>

#include "analog_turntable.h"
#include "config.h"
#include "rgb_manager.h"

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
  self->tt_react_hsv = DEFAULT_COLOUR;
  self->tt_breathing_hsv = DEFAULT_COLOUR;
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
      self->tt_react_hsv = DEFAULT_COLOUR;
      self->tt_breathing_hsv = DEFAULT_COLOUR;
      self->version++;
      update = true;
    default:
      break;
  }

  return update;
}

void config_init(config* self) {
  eeprom_read_block(self, CONFIG_BASE_ADDR, sizeof(*self));

  bool update;
  const uint16_t magic = eeprom_read_word(nullptr);
  if (magic != MAGIC) {
    update = true;
    eeprom_write_word(nullptr, MAGIC);
    init_config(self);
  } else {
    update = update_config(self);
  }

  if (update)
    eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
}

void config_update(uint8_t* addr, const uint8_t val) {
  eeprom_update_byte(addr, val);
}

callback toggle_reverse_tt(config* self) {
  self->reverse_tt ^= 1;
  eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);

  RgbManager::Turntable::reverse_tt(self->reverse_tt);
  timer_arm(&RgbManager::Turntable::combo_timer, CONFIG_CHANGE_NOTIFY_TIME);

  return callback{};
}

callback cycle_tt_effects(config* self) {
  self->tt_effect = TurntableMode((uint8_t(self->tt_effect) + 1) % uint8_t(TurntableMode::COUNT));
  eeprom_write_byte(CONFIG_TT_EFFECT_ADDR, uint8_t(self->tt_effect));

  RgbManager::Turntable::set_leds_off();

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

void display_tt_change(const uint8_t deadzone, const int range) {
  RgbManager::Turntable::display_tt_change(deadzone, range);
  timer_arm(&RgbManager::Turntable::combo_timer,
            CONFIG_CHANGE_NOTIFY_TIME);
}

#define DEADZONE_MAX 6
#define DEADZONE_MIN 1
callback increase_deadzone(config* self) {
  if (++self->tt_deadzone > DEADZONE_MAX) {
    self->tt_deadzone = DEADZONE_MAX;
  }
  eeprom_update_byte(CONFIG_TT_DEADZONE_ADDR, self->tt_deadzone);

  tt1.deadzone = self->tt_deadzone;

  display_tt_change(self->tt_deadzone, DEADZONE_MAX);

  return callback{};
}

callback decrease_deadzone(config* self) {
  if (--self->tt_deadzone < DEADZONE_MIN) {
    self->tt_deadzone = DEADZONE_MIN;
  }
  eeprom_update_byte(CONFIG_TT_DEADZONE_ADDR, self->tt_deadzone);

  tt1.deadzone = self->tt_deadzone;

  display_tt_change(self->tt_deadzone, DEADZONE_MAX);

  return callback{};
}

callback cycle_bar_effects(config* self) {
  do {
    self->bar_effect = BarMode((uint8_t(self->bar_effect) + 1) % uint8_t(BarMode::COUNT));
  } while (self->bar_effect == BarMode::PLACEHOLDER1 ||
           self->bar_effect == BarMode::PLACEHOLDER2 ||
           self->bar_effect == BarMode::PLACEHOLDER3 ||
           self->bar_effect == BarMode::PLACEHOLDER4);
  eeprom_write_byte(CONFIG_BAR_EFFECT_ADDR, uint8_t(self->bar_effect));

  RgbManager::Bar::set_leds_off();

  return callback{};
}

callback toggle_disable_led(config* self) {
  self->disable_led ^= 1;

  eeprom_write_byte(CONFIG_DISABLE_LED_ADDR, self->disable_led);

  if (self->disable_led)
    FastLED.clear(true);

  return callback{};
}

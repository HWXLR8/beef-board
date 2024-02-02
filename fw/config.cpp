#define CONFIG_VERSION 5

#define CONFIG_BASE_ADDR (uint8_t*)2
#define CONFIG_VERSION_ADDR CONFIG_BASE_ADDR
#define CONFIG_REVERSE_TT_ADDR (CONFIG_VERSION_ADDR + sizeof(config::version))
#define CONFIG_TT_EFFECT_ADDR (CONFIG_REVERSE_TT_ADDR + sizeof(config::reverse_tt))
#define CONFIG_TT_DEADZONE_ADDR (CONFIG_TT_EFFECT_ADDR + sizeof(config::tt_effect))
#define CONFIG_BAR_EFFECT_ADDR (CONFIG_TT_DEADZONE_ADDR + sizeof(config::tt_deadzone))
#define CONFIG_DISABLE_LED_ADDR (CONFIG_BAR_EFFECT_ADDR + sizeof(config::bar_effect))
#define CONFIG_TT_HSV_HUE_ADDR (CONFIG_DISABLE_LED_ADDR + sizeof(config::disable_led))
#define CONFIG_TT_HSV_SAT_ADDR (CONFIG_TT_HSV_HUE_ADDR + sizeof(config::tt_hsv.h))
#define CONFIG_TT_HSV_VAL_ADDR (CONFIG_TT_HSV_SAT_ADDR + sizeof(config::tt_hsv.s))

#define MAGIC 0xBEEF

#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdint.h>

#include "analog_turntable.h"
#include "config.h"

const auto default_colour = rgb2hsv_approximate(CRGB::Turquoise);

// initialize config with default values
void init_config(config* self) {
  self->version = CONFIG_VERSION;
  self->reverse_tt = 0;
  self->tt_effect = RgbManager::Turntable::Mode::SPIN;
  self->tt_deadzone = 4;
  self->bar_effect = RgbManager::Bar::Mode::HID;
  self->disable_led = 0;
  self->tt_hsv = HSV{ default_colour.h, 255, 255 };
}

bool update_config(config* self) {
  bool update = false;

  switch (self->version) {
  case 0:
    self->tt_effect = RgbManager::Turntable::Mode::SPIN;
    self->version++;
    update = true;
  case 1:
    self->tt_deadzone = 4;
    self->version++;
    update = true;
  case 2:
    self->bar_effect = RgbManager::Bar::Mode::HID;
    self->version++;
    update = true;
  case 3:
    self->disable_led = 0;
    self->version++;
    update = true;
  case 4:
    self->tt_hsv = HSV{ default_colour.h, 255, 255 };
    self->version++;
    update = true;
  }

  return update;
}

void config_init(config* self) {
  eeprom_read_block(self, CONFIG_BASE_ADDR, sizeof(*self));

  bool update = false;
  uint16_t magic = eeprom_read_word(0);
  if (magic != MAGIC) {
    update = true;
    eeprom_write_word(0, MAGIC);
    init_config(self);
  } else {
    update = update_config(self);
  }

  if (update) {
    eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
  }
}

void toggle_reverse_tt(config* self) {
  self->reverse_tt ^= 1;
  eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);

  RgbManager::Turntable::reverse_tt(self->reverse_tt);
  timer_arm(&RgbManager::Turntable::combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
}

void cycle_tt_effects(config* self) {
  using namespace RgbManager::Turntable;
  do {
    self->tt_effect = Mode((uint8_t(self->tt_effect) + 1) % uint8_t(Mode::COUNT));
  } while (self->tt_effect == Mode::PLACEHOLDER2 ||
           self->tt_effect == Mode::PLACEHOLDER3 ||
           self->tt_effect == Mode::PLACEHOLDER4 ||
           self->tt_effect == Mode::PLACEHOLDER5);
  eeprom_write_byte(CONFIG_TT_EFFECT_ADDR, uint8_t(self->tt_effect));

  RgbManager::Turntable::set_leds_off();
}

void tt_hsv_set_hue(config* self) {
  if (self->tt_effect != RgbManager::Turntable::Mode::STATIC)
    return;

  self->tt_hsv.h += tt1.raw_state;
}

void tt_hsv_update_hue(config* self) {
  eeprom_update_byte(CONFIG_TT_HSV_HUE_ADDR, self->tt_hsv.h);
}

void tt_hsv_set_sat(config* self) {
  if (self->tt_effect != RgbManager::Turntable::Mode::STATIC)
    return;

  auto tt1_report = tt1.raw_state;

  if ((self->tt_hsv.s == 0 && tt1_report < 0) ||
      (self->tt_hsv.s == 255 && tt1_report > 0)) // prevent looping around
    return;

  self->tt_hsv.s += tt1_report;
}

void tt_hsv_update_sat(config* self) {
  eeprom_update_byte(CONFIG_TT_HSV_SAT_ADDR, self->tt_hsv.s);
}

void tt_hsv_set_val(config* self) {
  if (self->tt_effect != RgbManager::Turntable::Mode::STATIC)
    return;

  auto tt1_report = tt1.raw_state;

  if ((self->tt_hsv.v == 0 && tt1_report < 0) ||
      (self->tt_hsv.v == 255 && tt1_report > 0)) // prevent looping around
    return;

  self->tt_hsv.v += tt1_report;
}

void tt_hsv_update_val(config* self) {
  eeprom_update_byte(CONFIG_TT_HSV_VAL_ADDR, self->tt_hsv.v);
}

void display_tt_change(uint8_t deadzone, int range) {
  RgbManager::Turntable::display_tt_change(deadzone, range);
  timer_arm(&RgbManager::Turntable::combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
}

#define DEADZONE_MAX 6
#define DEADZONE_MIN 1
void increase_deadzone(config* self) {
  if (++self->tt_deadzone > DEADZONE_MAX) {
    self->tt_deadzone = DEADZONE_MAX;
  }
  eeprom_update_byte(CONFIG_TT_DEADZONE_ADDR, self->tt_deadzone);

  tt1.deadzone = self->tt_deadzone;

  display_tt_change(self->tt_deadzone, DEADZONE_MAX);
}

void decrease_deadzone(config* self) {
  if (--self->tt_deadzone < DEADZONE_MIN) {
    self->tt_deadzone = DEADZONE_MIN;
  }
  eeprom_update_byte(CONFIG_TT_DEADZONE_ADDR, self->tt_deadzone);

  tt1.deadzone = self->tt_deadzone;

  display_tt_change(self->tt_deadzone, DEADZONE_MAX);
}

void cycle_bar_effects(config* self) {
  using namespace RgbManager::Bar;
  do {
    self->bar_effect = Mode((uint8_t(self->bar_effect) + 1) % uint8_t(Mode::COUNT));
  } while (self->bar_effect == Mode::PLACEHOLDER1 ||
           self->bar_effect == Mode::PLACEHOLDER2 ||
           self->bar_effect == Mode::PLACEHOLDER3 ||
           self->bar_effect == Mode::PLACEHOLDER4);
  eeprom_write_byte(CONFIG_BAR_EFFECT_ADDR, uint8_t(self->bar_effect));

  RgbManager::Bar::set_leds_off();
}

void toggle_disable_led(config* self) {
  self->disable_led ^= 1;

  eeprom_write_byte(CONFIG_DISABLE_LED_ADDR, self->disable_led);

  if (self->disable_led)
    FastLED.clear(true);
}

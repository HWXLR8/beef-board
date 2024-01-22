#define CONFIG_VERSION 3

#define CONFIG_BASE_ADDR (uint8_t*)2
#define CONFIG_VERSION_ADDR CONFIG_BASE_ADDR
#define CONFIG_REVERSE_TT_ADDR (CONFIG_VERSION_ADDR + sizeof(config::version))
#define CONFIG_TT_EFFECT_ADDR (CONFIG_REVERSE_TT_ADDR + sizeof(config::reverse_tt))
#define CONFIG_TT_DEADZONE_ADDR (CONFIG_TT_EFFECT_ADDR + sizeof(config::tt_effect))
#define CONFIG_BAR_EFFECT_ADDR (CONFIG_TT_DEADZONE_ADDR + sizeof(config::tt_deadzone))

#define MAGIC 0xBEEF

#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdint.h>

#include "analog_turntable.h"
#include "config.h"
#include "rgb_manager.h"

bool update_config(config* self);

void config_init(config* self) {
  eeprom_read_block(self, CONFIG_BASE_ADDR, sizeof(*self));

  bool update = false;
  uint16_t magic = eeprom_read_word(0);
  if (magic != MAGIC) {
    // initialize config with default values
    update = true;
    eeprom_write_word(0, MAGIC);

    self->version = CONFIG_VERSION;
    self->reverse_tt = 0;
    self->tt_effect = RgbManager::Turntable::SPIN;
    self->tt_deadzone = 4;
    self->bar_effect = RgbManager::Bar::HID;
  }

  update = update_config(self);

  if (update) {
    eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
  }
}

bool update_config(config* self) {
  bool update = false;

  switch (self->version) {
  case 0:
    self->tt_effect = RgbManager::Turntable::SPIN;
    self->version++;
  case 1:
    self->tt_deadzone = 4;
    self->version++;
  case 2:
    self->bar_effect = RgbManager::Bar::HID;
    self->version++;
    update = true;
  }

  return update;
}

void toggle_reverse_tt(config* self) {
  self->reverse_tt ^= 1;
  eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);

  // Illuminate + as blue, - as red in two halves
  int offset = self->reverse_tt ? 0 : RING_LIGHT_LEDS / 2;
  int blue_start = offset;
  int blue_end = blue_start + (RING_LIGHT_LEDS / 2);
  int red_start = (12 + offset) % RING_LIGHT_LEDS;
  int red_end = red_start + (RING_LIGHT_LEDS / 2);
  for (int i = blue_start; i < blue_end; ++i) {
    RgbManager::Turntable::leds[i] = CRGB::Blue;
  }
  for (int i = red_start; i < red_end; ++i) {
    RgbManager::Turntable::leds[i] = CRGB::Red;
  }
  timer_arm(&RgbManager::Turntable::combo_timer, CONFIG_CHANGE_NOTIFY_TIME);
}

void cycle_tt_effects(config* self) {
  do {
    self->tt_effect = RgbManager::Turntable::Mode((int(self->tt_effect) + 1) %
     RgbManager::Turntable::Mode::COUNT);
  } while (self->tt_effect == RgbManager::Turntable::Mode::PLACEHOLDER1 ||
           self->tt_effect == RgbManager::Turntable::Mode::PLACEHOLDER2 ||
           self->tt_effect == RgbManager::Turntable::Mode::PLACEHOLDER3 ||
           self->tt_effect == RgbManager::Turntable::Mode::PLACEHOLDER4 ||
           self->tt_effect == RgbManager::Turntable::Mode::PLACEHOLDER5);
  eeprom_write_byte(CONFIG_TT_EFFECT_ADDR, self->tt_effect);

  RgbManager::Turntable::set_leds_off();
}

void display_tt_change(uint8_t deadzone, int range) {
  int num_of_leds = deadzone * (RING_LIGHT_LEDS / range);
  for (int i = 0; i < num_of_leds; ++i) {
    RgbManager::Turntable::leds[i] = CRGB::Red;
  }
  for (int i = num_of_leds; i < RING_LIGHT_LEDS; ++i) {
    RgbManager::Turntable::leds[i] = CRGB::Black;
  }
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
  return; // Does nothing for now

  do {
    self->bar_effect = RgbManager::Bar::Mode((int(self->bar_effect) + 1) % RgbManager::Bar::Mode::COUNT);
  } while (self->bar_effect == RgbManager::Bar::Mode::PLACEHOLDER1 ||
           self->bar_effect == RgbManager::Bar::Mode::PLACEHOLDER2 ||
           self->bar_effect == RgbManager::Bar::Mode::PLACEHOLDER3 ||
           self->bar_effect == RgbManager::Bar::Mode::PLACEHOLDER4);
  eeprom_write_byte(CONFIG_BAR_EFFECT_ADDR, self->bar_effect);

  RgbManager::Bar::set_leds_off();
}

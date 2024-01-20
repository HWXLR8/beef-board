#define CONFIG_VERSION 2

#define CONFIG_BASE_ADDR (uint8_t*)2
#define CONFIG_VERSION_ADDR CONFIG_BASE_ADDR
#define CONFIG_REVERSE_TT_ADDR (CONFIG_VERSION_ADDR + sizeof(uint8_t))
#define CONFIG_TT_EFFECT_ADDR (CONFIG_REVERSE_TT_ADDR + sizeof(uint8_t))
#define CONFIG_TT_DEADZONE_ADDR (CONFIG_TT_EFFECT_ADDR + sizeof(ring_light_mode))

#define MAGIC 0xBEEF

#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdint.h>

#include "analog_turntable.h"
#include "config.h"
#include "tt_rgb_manager.h"

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
    self->tt_effect = SPIN;
    self->tt_deadzone = 4;
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
    self->tt_effect = SPIN;
    self->version++;
  case 1:
    self->tt_deadzone = 4;
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
    rgb(&tt_leds[i], 0, 0, 255);
  }
  for (int i = red_start; i < red_end; ++i) {
    rgb(&tt_leds[i], 255, 0, 0);
  }
  timer_arm(&combo_tt_led_timer, CONFIG_CHANGE_NOTIFY_TIME);
}

void cycle_tt_effects(config* self) {
  do {
    self->tt_effect = (self->tt_effect + 1) % NUM_OF_RING_LIGHT_MODES;
  } while (self->tt_effect == RING_LIGHT_MODE_PLACEHOLDER1 ||
           self->tt_effect == RING_LIGHT_MODE_PLACEHOLDER2 ||
           self->tt_effect == RING_LIGHT_MODE_PLACEHOLDER3 ||
           self->tt_effect == RING_LIGHT_MODE_PLACEHOLDER4 ||
           self->tt_effect == RING_LIGHT_MODE_PLACEHOLDER5);
  eeprom_write_byte(CONFIG_TT_EFFECT_ADDR, self->tt_effect);

  set_tt_leds_off();
}

void display_tt_change(uint8_t deadzone, int range) {
  int num_of_leds = deadzone * (RING_LIGHT_LEDS / range);
  for (int i = 0; i < num_of_leds; ++i) {
    rgb(&tt_leds[i], 255, 0, 0);
  }
  for (int i = num_of_leds; i < RING_LIGHT_LEDS; ++i) {
    rgb(&tt_leds[i], 0, 0, 0);
  }
  timer_arm(&combo_tt_led_timer, CONFIG_CHANGE_NOTIFY_TIME);
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

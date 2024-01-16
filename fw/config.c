#define CONFIG_VERSION 1
#define CONFIG_BASE_ADDR (uint8_t*)2
#define CONFIG_VERSION_ADDR CONFIG_BASE_ADDR
#define CONFIG_REVERSE_TT_ADDR (CONFIG_VERSION_ADDR + sizeof(uint8_t))
#define CONFIG_TT_EFFECT_ADDR (CONFIG_REVERSE_TT_ADDR + sizeof(uint8_t))

#define MAGIC 0xBEEF

#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdint.h>

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
    update = true;
    self->tt_effect = SPIN;
    self->version++;
  }

  return update;
}

void toggle_reverse_tt(config* self) {
  self->reverse_tt ^= 1;
  eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);
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

  set_led_off();
}

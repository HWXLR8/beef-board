#define CONFIG_VERSION 0
#define CONFIG_BASE_ADDR (uint8_t*)2
#define CONFIG_VERSION_ADDR (CONFIG_BASE_ADDR + 0)
#define CONFIG_REVERSE_TT_ADDR (CONFIG_BASE_ADDR + 1)

#define MAGIC 0xBEEF

#include <avr/eeprom.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

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
  }

  if (update) {
    eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
  }
}

void toggle_reverse_tt(config* self) {
  self->reverse_tt ^= 1;
  eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);
}

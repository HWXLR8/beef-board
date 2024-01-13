#define CONFIG_VERSION 0
#define CONFIG_BASE_ADDR (uint8_t*)0
#define CONFIG_VERSION_ADDR (uint8_t*)0
#define CONFIG_REVERSE_TT_ADDR (uint8_t*)1

#include <avr/eeprom.h>
#include <stdint.h>

#include "config.h"

void config_init(config* self) {
    eeprom_read_block(self, CONFIG_BASE_ADDR, sizeof(*self));

    if (self->version == 0xFF) {
        // initialize config with default values
        self->version = CONFIG_VERSION;
        self->reverse_tt = 0;

        eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
    }
}

void toggle_reverse_tt(config* self) {
    self->reverse_tt ^= 1;
    eeprom_write_byte(CONFIG_REVERSE_TT_ADDR, self->reverse_tt);
}

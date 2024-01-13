#include <stdint.h>

#include "config.h"

// this assumes that the pins to the 2 DATA lines are on the same port
// will need to refactor if this is not the case
// e.g. if a_pin is on F0 and b_pin is on D0
// PIN : [a_pin] : [b_pin] : [prev] : [tt_position]
tt_pins tt_x = { &PINF, 0, 1, -1, 0 };
// tt_pins tt_y = { &PIN?, ?, ?, -1, 0 };

void config_init(config* self) {
    eeprom_read_block(self, CONFIG_BASE_ADDR, sizeof(*self));

    if (self->version == 0xFF) {
        // initialize config with default values
        self->version = CONFIG_VERSION;
        self->reverse_tt = 0;

        eeprom_write_block(self, CONFIG_BASE_ADDR, sizeof(*self));
    }
}

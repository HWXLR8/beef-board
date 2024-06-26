#pragma once

#include "config.h"

struct combo {
  bool continuous;
  callback (*config_set)(config*);
};

extern combo (*get_button_combo_callback) (uint16_t);

void process_combos(config * current_config,
                    timer* combo_lights_timer);

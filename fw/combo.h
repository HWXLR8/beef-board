#pragma once

#include "config.h"

struct combo {
  bool continuous;
  callback (*config_set)(config*);
};

extern combo (*get_button_combo_callback) (uint16_t);
extern timer combo_lights_timer;

void process_combos();

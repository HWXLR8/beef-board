#pragma once

#include "config.h"

enum class Combo {
  REVERSE_TT       = BUTTON_1 | BUTTON_7 | BUTTON_8,
  TT_EFFECTS       = BUTTON_2 | BUTTON_8 | BUTTON_11,
  TT_DEADZONE_INCR = BUTTON_3 | BUTTON_8 | BUTTON_11,
  TT_DEADZONE_DECR = BUTTON_1 | BUTTON_8 | BUTTON_11,
  TT_RATIO_INCR    = BUTTON_7 | BUTTON_8 | BUTTON_11,
  TT_RATIO_DECR    = BUTTON_5 | BUTTON_8 | BUTTON_11,
  BAR_EFFECTS      = BUTTON_6 | BUTTON_8 | BUTTON_10,
  DISABLE_LED      = BUTTON_4 | BUTTON_8 | BUTTON_11,
  TT_HSV_HUE       = BUTTON_2 | BUTTON_11,
  TT_HSV_SAT       = BUTTON_4 | BUTTON_11,
  TT_HSV_VAL       = BUTTON_6 | BUTTON_11
};

void process_combos(config* current_config,
                    timer* combo_timer,
                    timer* combo_lights_timer);

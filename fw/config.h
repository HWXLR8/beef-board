// Copyright 2023 supervaka, HWXLR8

#pragma once

#include "pin.h"

#define CONFIG_ALL_HW_PIN { \
  CONFIG_HW_PIN(E0, E1),  /* BUTTON 1  */ \
  CONFIG_HW_PIN(B6, B7),  /* BUTTON 2  */ \
  CONFIG_HW_PIN(D4, D5),  /* BUTTON 3  */ \
  CONFIG_HW_PIN(D2, D3),  /* BUTTON 4  */ \
  CONFIG_HW_PIN(D0, D1),  /* BUTTON 5  */ \
  CONFIG_HW_PIN(D6, D7),  /* BUTTON 6  */ \
  CONFIG_HW_PIN(B4, B5),  /* BUTTON 7  */ \
  CONFIG_HW_PIN(C0, C1),  /* BUTTON 8  */ \
  CONFIG_HW_PIN(A6, A5),  /* BUTTON 9  */ \
  CONFIG_HW_PIN(C2, C3),  /* BUTTON 10 */ \
  CONFIG_HW_PIN(E2, A7),  /* BUTTON 11 */ \
}

// you still need to assign DDR and PORT manually in main() for now
// DDRF  &= 0b11111100;
// PORTF |= 0b00000011;

#define BUTTONS 11

#define TT_RATIO 2 // default 2:1 ratio

#define RING_LIGHT_LEDS 24
#define LIGHT_BAR_LEDS 16

typedef struct {
  uint8_t version;
  uint8_t reverse_tt;
} config;

void config_init(config* self);
void toggle_reverse_tt(config* self);

// button combos
#define REVERSE_TT_COMBO (BUTTON_1 | BUTTON_7 | BUTTON_8)

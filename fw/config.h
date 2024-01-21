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
  CONFIG_HW_PIN(E2, A7),  /* BUTTON 10 */ \
  CONFIG_HW_PIN(C2, C3),  /* BUTTON 11 */ \
}

// you still need to assign DDR and PORT manually in main() for now
// DDRF  &= 0b11111100;
// PORTF |= 0b00000011;

#define BUTTONS 11

#define TT_RATIO 2 // default 2:1 ratio

#define RING_LIGHT_LEDS 24
#define LIGHT_BAR_LEDS 16

#define CONFIG_CHANGE_NOTIFY_TIME 1000

// tentative names for LED ring modes
enum ring_light_mode {
  PLACEHOLDER1, // single colour
  SPIN,
  PLACEHOLDER2, // colour shift
  PLACEHOLDER3, // static rainbow
  PLACEHOLDER4, // reactive rainbow
  REACT_TO_SCR,
  PLACEHOLDER5, // breathing
  HID,
  COUNT
};
static_assert(sizeof(ring_light_mode) == sizeof(uint8_t));

// Do not reorder these fields
typedef struct {
  uint8_t version;
  uint8_t reverse_tt;
  ring_light_mode tt_effect;
  uint8_t tt_deadzone;
} config;

void config_init(config* self);
void toggle_reverse_tt(config* self);
void cycle_tt_effects(config* self);
void increase_deadzone(config* self);
void decrease_deadzone(config* self);

// button combos
#define NUM_OF_COMBOS 4
#define REVERSE_TT_COMBO (BUTTON_1 | BUTTON_7 | BUTTON_8)
#define TT_EFFECTS_COMBO (BUTTON_2 | BUTTON_8 | BUTTON_11)
#define TT_DEADZONE_INCR_COMBO (BUTTON_3 | BUTTON_8 | BUTTON_11)
#define TT_DEADZONE_DECR_COMBO (BUTTON_1 | BUTTON_8 | BUTTON_11)

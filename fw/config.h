// Copyright 2023 supervaka, HWXLR8

#include "beef.h"
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

// this assumes that the pins to the 2 DATA lines are on the same port
// will need to refactor if this is not the case
// e.g. if a_pin is on F0 and b_pin is on D0
// PIN : [a_pin] : [b_pin] : [prev] : [tt_position]
tt_pins tt_x = { &PINF, 0, 1, -1, 0 };
// tt_pins tt_y = { &PIN?, ?, ?, -1, 0 };

// you still need to assign DDR and PORT manually in main() for now
// DDRF  &= 0b11111100;
// PORTF |= 0b00000011;

#define TT_RATIO 2 // default 2:1 ratio

#define RING_LIGHT_LEDS 24
#define ring_light_port C // Data port
#define ring_light_pin 5 // Data out pin

// tentative names for LED ring modes
enum ring_light_mode {
  SPIN,
  REACT_TO_SCR
};

enum ring_light_mode ring_light_mode = SPIN;

#define LIGHT_BAR_LEDS 16
#define light_bar_port C // Data port
#define light_bar_pin 4 // Data out pin

enum light_bar_mode {
  FLASH,
  DISABLE,
  IDLE
};
enum ring_light_mode light_bar_mode = FLASH;
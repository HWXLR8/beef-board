// Copyright 2021 Dean Camera (dean [at] fourwalledcubicle [dot] com)
// Copyright 2023 supervaka, HWXLR8

#include "beef.h"
#include "pin_defs.h"


// https://stackoverflow.com/questions/24751387/can-i-comment-multi-line-macros
#define CONFIG_ALL_HW_PIN { \
  /* BUTTON_NO : INPUT_PIN : LED_PIN */ \
  CONFIG_HW_PIN(B0, B1),  /* BUTTON 1 : B0 : B1 */ \
  CONFIG_HW_PIN(B2, B3),  /* BUTTON 2 : B2 : B3 */ \
  CONFIG_HW_PIN(B4, B5),  /* BUTTON 3 : B4 : B5 */ \
  CONFIG_HW_PIN(B6, B7),  /* BUTTON 4 : B6 : B7 */ \
  CONFIG_HW_PIN(D0, D1),  /* BUTTON 5 : D0 : D1 */ \
  CONFIG_HW_PIN(D2, D3),  /* BUTTON 6 : D2 : D3 */ \
  CONFIG_HW_PIN(D4, D5),  /* BUTTON 7 : D4 : D5 */ \
  CONFIG_HW_PIN(D6, D7),  /* BUTTON 8 : D6 : D7 */ \
  CONFIG_HW_PIN(C0, C1),  /* BUTTON 9 : C0 : C1 */ \
  CONFIG_HW_PIN(C2, C3),  /* BUTTON 10: C2 : C3 */ \
  CONFIG_HW_PIN(C4, C5),  /* BUTTON 11: C4 : C5 */ \
}

hw_pin hw_pins[] = {
  { &DDRA, &PINA, &PORTA },
  { &DDRB, &PINB, &PORTB },
  { &DDRC, &PINC, &PORTC },
  { &DDRD, &PIND, &PORTD },
  { &DDRE, &PINE, &PORTE },
  { &DDRF, &PINF, &PORTF },
};

// need this indirection because preprocessor does not recognize "B0"
// as a macro that expands to two arguments, otherwise get:
// "error: too few arguments provided to function-like macro invocation"
#define CONFIG_HW_PIN(x, y) _CONFIG_HW_PIN(x, y)
#define _CONFIG_HW_PIN(input_port, input_pin, led_port, led_pin) { hw_pins[input_port##_], input_pin, hw_pins[led_port##_], led_pin }

// this assumes that the pins to the 2 DATA lines are on the same port
// will need to refactor if this is not the case
// e.g. if a_pin is on F0 and b_pin is on D0
// PIN : [a_pin] : [b_pin] : [prev] : [tt_position] 
tt_pins tt_x = { &PINF, 0, 1, -1, 0 };
// tt_pins tt_y = { &PIN?, ?, ?, -1, 0 };

// you still need to assign this manually in main() for now
// DDRF  &= 0b11111100; // turn this line into a parameterized macro
// PORTF |= 0b00000011;

// not sure if I want this on the top (usually macros seem to be on top)
// or on the bottom / middle (so that CONFIG_ALL_HW_PIN is at the very top)
#define A_ 0
#define B_ 1
#define C_ 2
#define D_ 3
#define E_ 4
#define F_ 5
#define G_ 6
#define H_ 7














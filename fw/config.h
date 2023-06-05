// Copyright 2021 Dean Camera (dean [at] fourwalledcubicle [dot] com)
// Copyright 2023 supervaka, HWXLR8

#include "beef.h"
#include "pin_defs.h"


// https://stackoverflow.com/questions/24751387/can-i-comment-multi-line-macros
#define CONFIG_ALL_HW_PIN { \
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
#define _CONFIG_HW_PIN(port_1, bit_1, port_2, bit_2) { hw_pins[port_1##_], bit_1, hw_pins[port_2##_], bit_2 }

// this assumes that the pins to the 2 DATA lines are on the same port
// will need to refactor if this is not the case
// e.g. if a_pin is on F0 and b_pin is on D0
// PIN : [a_pin] : [b_pin] : [prev] : [tt_position] 
tt_pins tt_x = { &PINF, 0, 1, -1, 0 };
// tt_pins tt_y = { &PIN?, ?, ?, -1, 0 };

// you still need to assign this manually in main() for now
// DDRF  &= 0b11111100; // turn this line into a parameterized macro
// PORTF |= 0b00000011;

// for some reason, it doesn't work when I omit this and try to use from pin_defs.h
// "error: too few arguments provided to function-like macro invocation"
#define A0 A, 0
#define A1 A, 1
#define A2 A, 2
#define A3 A, 3
#define A4 A, 4
#define A5 A, 5
#define A6 A, 6
#define A7 A, 7
#define B0 B, 0
#define B1 B, 1
#define B2 B, 2
#define B3 B, 3
#define B4 B, 4
#define B5 B, 5
#define B6 B, 6
#define B7 B, 7
#define C0 C, 0
#define C1 C, 1
#define C2 C, 2
#define C3 C, 3
#define C4 C, 4
#define C5 C, 5
#define C6 C, 6
#define C7 C, 7
#define D0 D, 0
#define D1 D, 1
#define D2 D, 2
#define D3 D, 3
#define D4 D, 4
#define D5 D, 5
#define D6 D, 6
#define D7 D, 7
#define E0 E, 0
#define E1 E, 1
#define E2 E, 2
#define E3 E, 3
#define E4 E, 4
#define E5 E, 5
#define E6 E, 6
#define E7 E, 7
#define F0 F, 0
#define F1 F, 1
#define F2 F, 2
#define F3 F, 3
#define F4 F, 4
#define F5 F, 5
#define F6 F, 6
#define F7 F, 7
#define G0 G, 0
#define G1 G, 1
#define G2 G, 2
#define G3 G, 3
#define G4 G, 4
#define G5 G, 5
#define G6 G, 6
#define G7 G, 7
#define H0 H, 0
#define H1 H, 1
#define H2 H, 2
#define H3 H, 3
#define H4 H, 4
#define H5 H, 5
#define H6 H, 6
#define H7 H, 7

#define A_ 0
#define B_ 1
#define C_ 2
#define D_ 3
#define E_ 4
#define F_ 5
#define G_ 6
#define H_ 7


/* moved to main() in beef.c */
// hw_pin hw_pins[] = {
//   { &DDRA, &PINA, &PORTA },
//   { &DDRB, &PINB, &PORTB },
//   { &DDRC, &PINC, &PORTC },
//   { &DDRD, &PIND, &PORTD },
//   { &DDRE, &PINE, &PORTE },
//   { &DDRF, &PINF, &PORTF },
// };


// button_pins buttons[] = {
//   CONFIG_HW_PIN(B0, B1),  // BUTTON 1 : B0 : B1  
//   CONFIG_HW_PIN(B2, B3),  // BUTTON 2 : B2 : B3  
//   CONFIG_HW_PIN(B4, B5),  // BUTTON 3 : B4 : B5  
//   CONFIG_HW_PIN(B6, B7),  // BUTTON 4 : B6 : B7  
//   CONFIG_HW_PIN(D0, D1),  // BUTTON 5 : D0 : D1  
//   CONFIG_HW_PIN(D2, D3),  // BUTTON 6 : D2 : D3  
//   CONFIG_HW_PIN(D4, D5),  // BUTTON 7 : D4 : D5  
//   CONFIG_HW_PIN(D6, D7),  // BUTTON 8 : D6 : D7  
//   CONFIG_HW_PIN(C0, C1),  // BUTTON 9 : C0 : C1  
//   CONFIG_HW_PIN(C2, C3),  // BUTTON 10: C2 : C3
//   CONFIG_HW_PIN(C4, C5),  // BUTTON 11: C4 : C5 
// };












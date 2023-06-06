// Copyright 2023 supervaka, HWXLR8

#include "beef.h"

hw_pin hw_pins[] = {
  { &DDRA, &PINA, &PORTA },
  { &DDRB, &PINB, &PORTB },
  { &DDRC, &PINC, &PORTC },
  { &DDRD, &PIND, &PORTD },
  { &DDRE, &PINE, &PORTE },
  { &DDRF, &PINF, &PORTF },
};

// to index into hw_pins[] array
#define A_ 0
#define B_ 1
#define C_ 2
#define D_ 3
#define E_ 4
#define F_ 5

// need this indirection because preprocessor does not recognize "B0"
// as a macro that expands to two arguments, otherwise get:
// "error: too few arguments provided to function-like macro invocation"
#define CONFIG_HW_PIN(x, y) _CONFIG_HW_PIN(x, y)
#define _CONFIG_HW_PIN(input_port, input_pin, led_port, led_pin) { hw_pins[input_port##_], input_pin, hw_pins[led_port##_], led_pin }

#define CONFIG_DDR_INPUT(DDR, pin_number) (*(DDR) &= ~(1<<pin_number))
#define CONFIG_DDR_LED(DDR, pin_number) (*(DDR) |= (1<<pin_number))

#define CONFIG_PORT_INPUT(PORT, pin_number) (*(PORT) |= (1<<pin_number))
#define CONFIG_PORT_LED(PORT, pin_number) (*(PORT) &= ~(1<<pin_number))

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
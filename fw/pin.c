#include "pin.h"

hw_pin hw_pins[] = {
  { &DDRA, &PINA, &PORTA },
  { &DDRB, &PINB, &PORTB },
  { &DDRC, &PINC, &PORTC },
  { &DDRD, &PIND, &PORTD },
  { &DDRE, &PINE, &PORTE },
  { &DDRF, &PINF, &PORTF },
};

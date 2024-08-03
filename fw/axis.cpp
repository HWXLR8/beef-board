#include "axis.h"
#include "config.h"

int8_t tt_transitions[4][4];
AnalogAxis analog_x(PINF5);
AnalogAxis analog_y(PINF4);
QeAxis tt_x(&PINF, PINF0, PINF1);
QeAxis tt_y(&PINF, PINF2, PINF3);

AnalogAxis::AnalogAxis(uint8_t pin) : pin(pin){}

void AnalogAxis::poll() {
  // Select ADC channel
  ADMUX = (ADMUX & 0xF8) | (pin & 0x07);
  // Start conversion
  ADCSRA |= (1 << ADSC);
  // Wait for conversion to finish
  while (ADCSRA & (1 << ADSC));
  // Read 8-bits of ADC value
  position = ADCH;
}

uint8_t AnalogAxis::get() const {
  return position;
}

QeAxis::QeAxis(volatile uint8_t* PIN, uint8_t a_pin, uint8_t b_pin) : PIN(PIN), a_pin(a_pin), b_pin(b_pin){}

void QeAxis::poll() {
  // tt logic
  // example where tt_x wired to F0/F1:
  // curr is binary number ab
  // where a is the signal of F0
  // and b is the signal of F1
  // therefore when F0 == 1 and F1 == 0, then curr == 0b10
  const uint8_t a = (*PIN >> a_pin) & 1;
  const uint8_t b = (*PIN >> b_pin) & 1;
  const uint8_t curr = (a << 1) | b;

  const auto direction = tt_transitions[prev][curr];
  position += direction;
  position %= 256 * current_config.tt_ratio;
  prev = curr;
}

uint8_t QeAxis::get() const {
  return position / current_config.tt_ratio;
}

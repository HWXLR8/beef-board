#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "timer.h"

static unsigned long millis(void) {
  return milliseconds;
}

static unsigned long micros(void) {
  uint8_t oldSREG = SREG;
  cli();
  uint32_t m = milliseconds;
  uint8_t t = TCNT0;

  if ((TIFR0 & _BV(TOV0)) && t < 255)
    m++;

  SREG = oldSREG;

  // Accurate within 4us due to the prescaler
  return m * 1000 + t * (64 / (F_CPU / 1000000));
}

static void delay(uint32_t ms) {
  _delay_ms(ms);
}

#pragma GCC diagnostic pop

#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#include <util/delay.h>

static unsigned long millis(void) {
  return 0;
}

static unsigned long micros(void) {
  return 0;
}

static void delay(uint32_t ms) {
  _delay_ms(ms);
}

#pragma GCC diagnostic pop

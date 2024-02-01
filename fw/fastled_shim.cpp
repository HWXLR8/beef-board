#include "FastLED/src/platforms/avr/led_sysdefs_avr.h"

#include "timer.h"

volatile unsigned long timer0_millis __attribute__((unused));

uint32_t get_millisecond_timer(void) {
  return milliseconds;
}

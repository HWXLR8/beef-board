#pragma once

#include "timer.h"

typedef struct {
  // Number of ticks we need to advance before recognizing an input
  uint8_t deadzone;
  // How long to sustain the input before clearing it (if opposite direction is input, we'll release immediately)
  uint32_t sustain_ms;
  // Always provide a zero-input for one poll before reversing?
  bool clear;

  // State: Center of deadzone
  uint32_t center;
  bool center_valid;

  // time to: reset center to counter
  timer sustain_timer;  

  int8_t state; // -1, 0, 1
} analog_turntable;
analog_turntable tt1;

void analog_turntable_init(analog_turntable* self, uint8_t deadzone, uint32_t sustain_ms, bool clear);
int8_t analog_turntable_poll(analog_turntable* self, uint32_t current_value);

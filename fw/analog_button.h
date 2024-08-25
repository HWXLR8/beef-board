#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "timer.h"

typedef struct {
  // Number of ticks we need to advance before recognizing an input
  uint8_t deadzone;
  // How long to sustain the input before clearing it (if opposite direction is input, we'll release immediately)
  uint8_t sustain_ms;
  // Always provide a zero-input for one poll before reversing?
  bool clear;

  // State: Center of deadzone
  uint32_t center;
  bool center_valid;

  // time to: reset center to counter
  timer sustain_timer;

  int8_t state; // -1, 0, 1
  int8_t direction; // currently observed direction
} analog_button;
extern analog_button button_x;
extern analog_button button_y;

void analog_button_init(analog_button* self, uint8_t deadzone, uint8_t sustain_ms, bool clear);
int8_t analog_button_poll(analog_button* self, uint32_t current_value);
#ifdef __cplusplus
}
#endif

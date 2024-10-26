#pragma once

#include "timer.h"

class AnalogButton {
public:
  AnalogButton() = default;

  void init(uint8_t deadzone, bool clear, uint8_t current_value);
  int8_t poll(uint8_t deadzone, uint8_t sustain_ms, uint8_t current_value);

  int8_t direction; // currently observed direction
private:
  // Number of ticks we need to advance before recognizing an input
  uint8_t current_deadzone;
  // Always provide a zero-input for one poll before reversing?
  bool clear;

  // State: Center of deadzone
  uint8_t center;

  // time to: reset center to counter
  timer sustain_timer;

  int8_t state; // -1, 0, 1
};
extern AnalogButton button_x;
extern AnalogButton button_y;

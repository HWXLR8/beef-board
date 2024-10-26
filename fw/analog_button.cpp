#include "analog_button.h"

AnalogButton button_x;
AnalogButton button_y;

void AnalogButton::init(uint8_t deadzone, bool clear, uint8_t current_value) {
  current_deadzone = deadzone;
  this->clear = clear;
  center = current_value;
  timer_arm(&sustain_timer, 0);
}

int8_t AnalogButton::poll(uint8_t deadzone, uint8_t sustain_ms, const uint8_t current_value) {
  int8_t delta = current_value - center;
  // Handle wrap-around cases
  if (delta > 127) {
    delta = (delta - 256) * -1;
  } else if (delta < -127) {
    delta = (delta + 256) * -1;
  }

  // is the current value sufficiently far away from the center?
  int8_t new_direction = 0;
  if (delta >= (int8_t)current_deadzone) {
    new_direction = 1;
  } else if (delta <= -(int8_t)current_deadzone) {
    new_direction = -1;
  }

  auto direction_change = false;
  if (new_direction != 0) {
    // encoder is moving
    // keep updating the new center, and keep extending the sustain timer
    center = current_value;
    state = new_direction;
    timer_arm(&sustain_timer, sustain_ms);
    // disable deadzone to increase sensitivity to direction changes
    deadzone = 1;
    direction_change = direction == -state;
  } else if (timer_is_expired(&sustain_timer)) {
    // sustain timer expired, time to reset to neutral
    state = 0;
  }
  current_deadzone = deadzone;
  direction = new_direction;

  if (direction_change && clear) {
    return 0;
  }
  return state;
}

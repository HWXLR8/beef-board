#pragma once

#include "timer.h"

// Doesn't actually track BPM, but it's the closest concept
class Bpm {
public:
  explicit Bpm(uint8_t max_level);

  uint8_t update(uint16_t button_state);

private:
  uint8_t max_level;
  timer decay{};
  timer button_guard{};
  uint8_t current_level{};
  uint16_t last_button_state{};
};

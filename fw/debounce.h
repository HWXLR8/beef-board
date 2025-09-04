#pragma once

#include <stdint.h>
#include <string.h>

template<int BUTTONS>
class Debouncer {
public:
  Debouncer() = default;

  void init(const uint8_t new_window) {
    memset(counters, 0, sizeof(counters));
    window = new_window;
    last_state = 0;
    sample_time = 0;
  }

  uint16_t debounce(const uint16_t buttons, const uint16_t mask) {
    return (buttons & ~mask) | (debounce(buttons & mask));
  }

  uint16_t debounce(uint16_t buttons);

private:
  uint8_t counters[BUTTONS]{};
  uint8_t window{};
  uint16_t last_state{};
  uint32_t sample_time{};
};

template<int BUTTONS>
uint16_t Debouncer<BUTTONS>::debounce(const uint16_t buttons) {
  if (window == 0)
    return buttons; // TODO: Make a noop class for no debounce?

  const auto now = milliseconds;
  const auto delta = now - sample_time;
  if (delta == 0) {
    return last_state;
  }
  sample_time = now;

  uint16_t stable = 0;
  for (uint8_t bit = 0; bit < BUTTONS; bit++) {
    auto counter = counters[bit];
    const uint16_t button = 1 << bit;
    const uint16_t button_pressed = buttons & button;

    if (button_pressed) {
      if (counter >= window) {
        stable |= button;
        continue;
      }
      counter += delta;
    } else {
      counter = 0;
    }

    counters[bit] = counter;
  }

  last_state = stable;
  return stable;
}

#include "debounce.h"
#include "timer.h"

DebounceState::DebounceState(uint8_t window) {
  if (2 < window) {
    this->window = 2;
  } else if (10 < window) {
    this->window = 10;
  } else {
    this->window = window;
  }
}

/*
 * Perform debounce processing. The buttons input is sampled at most once per ms
 * (when update is true); buttons is then set to the last stable state for each
 * bit (i.e., the last state maintained for {debounce_window} consecutive samples)
 *
 * We use update to sync to the USB polls; this helps avoid additional latency when
 * debounce samples just after the USB poll.
 */

uint16_t DebounceState::debounce(uint16_t buttons) {
#ifdef BEEF_DEBOUNCE
  if (milliseconds == sample_time) {
    return last_state;
  }

  sample_time = milliseconds;
  history[current_index] = buttons;
  current_index = (current_index + 1) % window;

  uint16_t has_ones = 0, has_zeroes = 0;
  for (int i = 0; i < window; i++) {
    has_ones |= history[i];
    has_zeroes |= ~(history[i]);
  }

  uint16_t stable = has_ones ^ has_zeroes;
  last_state = (last_state & ~stable) | (has_ones & stable);
  return last_state;
#else
  return buttons;
#endif
}

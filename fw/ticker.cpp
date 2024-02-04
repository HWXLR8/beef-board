#include "ticker.h"

Ticker::Ticker(uint32_t tick_duration) : tick_duration(tick_duration){};

uint32_t Ticker::get_ticks() {
  static uint32_t last_update_time = 0;

  auto now = milliseconds;
  uint32_t delta_time = now - last_update_time;
  uint32_t ticks = delta_time / tick_duration;
  if (ticks > 0) {
    last_update_time = now;
  }

  return ticks;
}

#include <LUFA/Common/Common.h>

#include "ticker.h"
#include "timer.h"

Ticker::Ticker(uint8_t tick_duration, uint16_t total_duration) :
  tick_duration(tick_duration),
  last_tick_time(milliseconds),
  tick_limit(total_duration / tick_duration){}

uint32_t Ticker::get_ticks() {
  if (tick_limit != 0 && current_ticks >= tick_limit) {
    return 0;
  }

  if (first_tick) {
    first_tick = false;
    current_ticks++;
    return 1;
  }

  const auto now = milliseconds;
  const uint32_t delta_time = now - last_tick_time;
  const uint8_t ticks = delta_time / tick_duration;
  if (ticks > 0) {
    last_tick_time = get_nearest_tick(now);
    current_ticks += ticks;
    if (tick_limit > 0) {
      MIN(tick_limit, current_ticks);
    }
  }

  return ticks;
}

void Ticker::reset() {
  if (tick_limit == 0) {
    return;
  }

  last_tick_time = milliseconds;
  current_ticks = 0;
}

void Ticker::reset(const uint8_t tick_duration, const uint16_t total_duration) {
  reset();
  this->tick_duration = tick_duration;
  tick_limit = total_duration / tick_duration;
}

uint32_t Ticker::get_nearest_tick(uint32_t now) const {
  // Round down to nearest tick
  return now - (now % tick_duration);
}

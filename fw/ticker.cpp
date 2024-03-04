#include "ticker.h"
#include "timer.h"

Ticker::Ticker(uint32_t tick_duration, uint32_t total_duration) :
  tick_duration(tick_duration),
  last_tick_time(milliseconds),
  tick_limit(total_duration / tick_duration){}

uint32_t Ticker::get_ticks() {
  if (tick_limit != 0 && current_ticks >= tick_limit)
    return 0;

  auto now = milliseconds;
  uint32_t delta_time = now - last_tick_time;
  uint32_t ticks = delta_time / tick_duration;
  if (ticks > 0) {
    last_tick_time = get_nearest_tick(now);
    current_ticks += ticks;
    int32_t over_limit = current_ticks - tick_limit;
    if (tick_limit != 0 && over_limit > 0) {
      ticks -= over_limit;
    }
  }

  return ticks;
}

void Ticker::reset() {
  if (tick_limit == 0)
    return;

  auto now = milliseconds;
  last_tick_time = now;
  current_ticks = 0;
}

uint32_t Ticker::get_nearest_tick(uint32_t now) const {
  // Round down to nearest tick
  return now - (now % tick_duration);
}

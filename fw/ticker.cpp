#include "ticker.h"
#include "timer.h"

Ticker::Ticker(uint8_t tick_duration, uint16_t total_duration) :
  tick_duration(tick_duration),
  tick_limit(total_duration / tick_duration){}

uint8_t Ticker::get_ticks() {
  if (tick_limit != 0 && current_ticks >= tick_limit) {
    return 0;
  }

  const auto now = milliseconds;
  if (first_tick) {
    last_tick_time = now;
  }

  const uint32_t delta_time = now - last_tick_time;
  uint8_t ticks = delta_time / tick_duration;

  last_tick_time += ticks * tick_duration;
  ticks += static_cast<uint8_t>(first_tick);
  current_ticks += ticks;
  if (tick_limit != 0 && current_ticks > tick_limit) {
    ticks -= current_ticks - tick_limit;
  }

  first_tick = false;
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

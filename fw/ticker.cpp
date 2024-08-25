#include "ticker.h"
#include "timer.h"

Ticker::Ticker(uint8_t tick_duration) : tick_duration(tick_duration){}

uint8_t Ticker::get_ticks() {
  if (first_tick) {
    reset();
  }

  const auto now = milliseconds;
  const uint16_t delta_time = now - last_tick_time;
  uint8_t ticks = delta_time / tick_duration;

  last_tick_time += ticks * tick_duration;
  ticks += static_cast<uint8_t>(first_tick);

  first_tick = false;
  return ticks;
}

void Ticker::reset() {
  last_tick_time = milliseconds;
}

void Ticker::reset(const uint8_t tick_duration) {
  reset();
  this->tick_duration = tick_duration;
}

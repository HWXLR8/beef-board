#include <FastLED/src/FastLED.h>

#include "rgb_patterns.h"

// Cycle from zero to full-bright to zero in around 2 seconds
BreathingPattern::BreathingPattern() : ticker(Ticker(8)){}

BreathingPattern::BreathingPattern(uint16_t duration,
                                   uint16_t cycle_time) :
  cycle_time(cycle_time),
  ticker(Ticker(8, duration)) {
  timer_arm(&breathing_timer, cycle_time);
}

uint8_t BreathingPattern::update() {
  if (cycle_time > 0 && !timer_is_active(&breathing_timer)) {
    ticker.reset();
    timer_arm(&breathing_timer, cycle_time);
  }

  auto ticks = ticker.get_ticks();
  if (ticks > 0) {
    theta += ticks;
    v = quadwave8(theta);
  }

  return v;
}

SpinPattern::SpinPattern(const uint8_t spin_duration,
                         const uint8_t fast_spin_duration,
                         const uint8_t limit) :
  spin_duration(spin_duration),
  fast_spin_duration(fast_spin_duration),
  limit(limit),
  ticker(Ticker(spin_duration)){}

uint8_t SpinPattern::update(int8_t tt_report) {
  if (last_tt_report != tt_report) {
    auto duration = tt_report != 0 ?
                    fast_spin_duration :
                    spin_duration;
    ticker.reset(duration);
    last_tt_report = tt_report;
  }

  auto ticks = ticker.get_ticks();
  if (ticks > 0) {
    if (tt_report == -1) {
      if (spin_counter < ticks) {
        spin_counter = limit - 1 - (ticks - spin_counter);
      } else {
        spin_counter -= ticks;
      }
    } else {
      spin_counter = (spin_counter + ticks) % limit;
    }
  }

  return spin_counter;
}

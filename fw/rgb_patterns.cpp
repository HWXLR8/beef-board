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

bool SpinPattern::update(const int8_t tt_report) {
  if (last_tt_report != tt_report) {
    auto duration = tt_report != 0 ?
                    fast_spin_duration :
                    spin_duration;
    ticker.reset(duration);
    last_tt_report = tt_report;
  }

  const auto ticks = ticker.get_ticks();
  if (tt_report == -1) {
    spin_counter += limit - ticks;
  } else {
    spin_counter += ticks;
  }
  spin_counter %= limit;

  return ticks > 0;
}

uint8_t SpinPattern::get() const {
  return spin_counter;
}

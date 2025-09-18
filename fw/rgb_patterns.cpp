#include <FastLED/src/FastLED.h>

#include "rgb_patterns.h"

// Cycle from zero to full-bright to zero in around 2 seconds
BreathingPattern::BreathingPattern() : ticker(Ticker(8)){}

BreathingPattern::BreathingPattern(const uint16_t cycle_time) :
  cycle_time(cycle_time),
  ticker(Ticker(8)) {
  timer_arm(&breathing_timer, cycle_time);
}

uint8_t BreathingPattern::update() {
  if (timer_is_expired(&breathing_timer)) {
    ticker.reset();
    timer_arm(&breathing_timer, cycle_time);
    theta = 0;
  }

  const auto ticks = ticker.get_ticks();
  if (ticks > 0) {
    theta += ticks;
    if (cycle_time != 0 && theta < ticks) {
      // Overflow, cap at full cycle
      theta = 255;
      return v;
    }
    v = quadwave8(theta);
  }

  return v;
}

SpinPattern::SpinPattern(){}

SpinPattern::SpinPattern(const uint8_t spin_duration,
                         const uint8_t fast_spin_duration,
                         const uint8_t limit) :
  spin_duration(spin_duration),
  fast_spin_duration(fast_spin_duration),
  limit(limit),
  ticker(Ticker(spin_duration)){}

void SpinPattern::init(const uint8_t spin_duration,
                       const uint8_t fast_spin_duration,
                       const uint8_t limit) {
  this->spin_duration = spin_duration;
  this->fast_spin_duration = fast_spin_duration;
  this->limit = limit;
  this->ticker.init(spin_duration);
}

bool SpinPattern::update(const int8_t tt_report) {
  if (last_tt_report != tt_report) {
    const auto duration = tt_report != 0 ?
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

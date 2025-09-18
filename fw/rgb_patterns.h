#pragma once

#include "ticker.h"

class BreathingPattern {
public:
  BreathingPattern();
  explicit BreathingPattern(uint16_t cycle_time);

  uint8_t update();

private:
  uint16_t cycle_time = 0;
  Ticker ticker;
  ::timer breathing_timer{};
  uint8_t theta = 0;
  uint8_t v = 0;
};

class SpinPattern {
public:
  SpinPattern();
  SpinPattern(uint8_t spin_duration,
              uint8_t fast_spin_duration,
              uint8_t limit = 0);

  void init(uint8_t spin_duration,
            uint8_t fast_spin_duration,
            uint8_t limit);
  bool update(int8_t tt_report);
  uint8_t get() const;

private:
  uint8_t spin_duration;
  uint8_t fast_spin_duration;
  int8_t last_tt_report = 0;
  uint8_t spin_counter = 0;
  uint8_t limit;
  Ticker ticker{};
};

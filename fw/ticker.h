#pragma once

#include "timer.h"

class Ticker {
  public:
    Ticker(uint32_t tick_duration, uint32_t total_duration = 0);

    uint32_t get_ticks();
    void reset();

  private:
    uint32_t tick_duration;
    uint32_t last_tick_time;
    uint32_t tick_limit;
    uint32_t current_ticks;

    uint32_t get_nearest_tick(uint32_t now);
};

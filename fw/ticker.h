#pragma once

#include "timer.h"

class Ticker {
  public:
    Ticker(uint32_t tick_duration);

    uint32_t get_ticks();

  private:
    uint32_t tick_duration;
};

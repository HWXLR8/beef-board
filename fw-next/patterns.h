#pragma once

#include "ticker.h"
#include "timer.h"

class BreathingPattern
{
public:
    BreathingPattern();
    explicit BreathingPattern(uint16_t cycle_time);

    uint8_t update();

private:
    uint16_t cycle_time = 0;
    Ticker ticker;
    timer_t breathing_timer{};
    uint8_t theta = 0;
    uint8_t v = 0;
    int8_t direction = 1;
};

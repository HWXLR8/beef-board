#pragma once

#include <cstdint>

class Ticker
{
public:
    explicit Ticker(uint32_t tick_duration);

    uint32_t get_ticks();
    void reset();
    void reset(uint32_t tick_duration);

private:
    uint8_t tick_duration;
    uint32_t last_tick_time = 0;
    bool first_tick = true;
};

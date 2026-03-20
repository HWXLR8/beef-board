#pragma once

#include <cstdint>

#include "pico/time.h"

class Limiter
{
public:
    Limiter(uint16_t frame_time_us) : frame_time_us(frame_time_us)
    {
    }

    bool ready()
    {
        const auto now = to_us_since_boot(get_absolute_time());
        if (now - last_show_us < frame_time_us) return false;
        last_show_us = now;
        return true;
    }

private:
    uint16_t frame_time_us;
    uint64_t last_show_us = 0;
};

#include "patterns.h"

#include "fastled.h"

// Cycle from zero to full-bright to zero in around 2 seconds
BreathingPattern::BreathingPattern() : ticker(Ticker(4))
{
}

BreathingPattern::BreathingPattern(const uint16_t cycle_time) :
    cycle_time(cycle_time),
    ticker(Ticker(4))
{
    breathing_timer.arm(cycle_time);
}

uint8_t BreathingPattern::update()
{
    if (breathing_timer.is_expired())
    {
        ticker.reset();
        breathing_timer.arm(cycle_time);
        theta = 0;
    }

    const auto ticks = ticker.get_ticks();
    if (ticks > 0)
    {
        auto old_theta = theta;
        theta += ticks * direction;

        switch (direction)
        {
        case 1:
            if (theta < ticks) // overflow
            {
                theta = 255 - theta - ticks;
                direction = -1;
            }
            break;
        case -1:
            if (255 - theta < ticks) // underflow
            {
                if (cycle_time != 0)
                {
                    // stop pattern after one cycle
                    theta = 0;
                    return 0;
                }
                theta = ticks - old_theta;
                direction = 1;
            }
            break;
        default: ;
        }

        v = quadwave8(theta);
    }

    return v;
}

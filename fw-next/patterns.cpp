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
        direction = 1;
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

SpinPattern::SpinPattern(const uint8_t spin_duration,
                         const uint8_t fast_spin_duration,
                         const uint8_t limit) :
    spin_duration(spin_duration),
    fast_spin_duration(fast_spin_duration),
    limit(limit),
    ticker(new Ticker(spin_duration))
{
}

bool SpinPattern::update(const int8_t tt_report)
{
    if (last_tt_report != tt_report)
    {
        const auto duration = tt_report != 0 ? fast_spin_duration : spin_duration;
        ticker->reset(duration);
        last_tt_report = tt_report;
    }

    const auto ticks = ticker->get_ticks();
    if (tt_report == -1)
    {
        spin_counter += limit - ticks;
    }
    else
    {
        spin_counter += ticks;
    }
    spin_counter %= limit;

    return ticks > 0;
}

uint8_t SpinPattern::get() const
{
    return spin_counter;
}

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

constexpr auto DECAY_TIME = 30;
// This effectively controls how easy it is to reach max_level
constexpr auto BUTTON_GUARD_TIME = 15;

Bpm::Bpm(const uint8_t max_level) : max_level(max_level)
{
}

uint8_t Bpm::update(const uint16_t button_state)
{
    if (!limiter.ready())
    {
        return current_level;
    }

    if (!button_guard.is_active())
    {
        const bool pressed_or_released = button_state ^ last_button_state;
        if (pressed_or_released)
        {
            current_level = MIN(current_level+1, max_level);
            decay.arm(DECAY_TIME);
            button_guard.arm(BUTTON_GUARD_TIME);
        }
    }

    if (current_level > 0 && decay.is_expired())
    {
        --current_level;
        decay.arm(DECAY_TIME);
    }

    last_button_state = button_state;

    return current_level;
}

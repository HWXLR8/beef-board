#include "ticker.h"
#include "timer.h"

Ticker::Ticker(const uint32_t tick_duration) : tick_duration(tick_duration)
{
}

uint32_t Ticker::get_ticks()
{
    if (first_tick)
    {
        reset();
    }

    const auto now = board_millis();
    const auto delta_time = now - last_tick_time;
    auto ticks = delta_time / tick_duration;

    last_tick_time += ticks * tick_duration;
    ticks += static_cast<uint8_t>(first_tick);

    first_tick = false;
    return ticks;
}

void Ticker::reset()
{
    last_tick_time = board_millis();
}

void Ticker::reset(const uint32_t tick_duration)
{
    reset();
    this->tick_duration = tick_duration;
}

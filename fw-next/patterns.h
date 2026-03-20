#pragma once

#include "limiter.h"
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

class SpinPattern
{
public:
    SpinPattern(uint8_t spin_duration,
                uint8_t fast_spin_duration,
                uint8_t limit = 0);

    bool update(int8_t tt_report);
    [[nodiscard]] uint8_t get() const;

private:
    uint8_t spin_duration;
    uint8_t fast_spin_duration;
    int8_t last_tt_report = 0;
    uint8_t spin_counter = 0;
    uint8_t limit;
    Ticker* ticker;
};

// Doesn't actually track BPM, but it's the closest concept
class Bpm
{
public:
    explicit Bpm(uint8_t max_level);

    uint8_t update(uint16_t button_state);

private:
    uint8_t max_level;
    timer_t decay{};
    timer_t button_guard{};
    uint8_t current_level{};
    uint16_t last_button_state{};
    Limiter limiter = Limiter(1000);
};

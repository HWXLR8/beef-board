#pragma once

#include "limiter.h"
#include "timer.h"

class AnalogButton
{
public:
    AnalogButton(uint8_t deadzone, bool clear) : current_deadzone(deadzone), clear(clear)
    {
    }

    int8_t poll(uint8_t current_value);

    int8_t direction = 0; // currently observed direction
private:
    // Number of ticks we need to advance before recognizing an input
    uint8_t current_deadzone;
    // Always provide a zero-input for one poll before reversing?
    bool clear;

    // State: Center of deadzone
    uint8_t center = 0;

    // time to: reset center to counter
    timer_t sustain_timer;

    int8_t state = 0; // -1, 0, 1

    Limiter limiter = Limiter(1000);
};

extern AnalogButton* button_x;
extern AnalogButton* button_y;

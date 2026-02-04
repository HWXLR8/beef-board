#pragma once
#include <cstdint>

extern uint16_t button_state;
extern bool reactive_leds;

void update_tt_transitions();

// button macros to map to bit positions within button_state
enum
{
    BUTTON_1 = 1 << 0,
    BUTTON_2 = 1 << 1,
    BUTTON_3 = 1 << 2,
    BUTTON_4 = 1 << 3,
    BUTTON_5 = 1 << 4,
    BUTTON_6 = 1 << 5,
    BUTTON_7 = 1 << 6,
    BUTTON_8 = 1 << 7,
    BUTTON_9 = 1 << 8,
    BUTTON_10 = 1 << 9,
    BUTTON_11 = 1 << 10
};

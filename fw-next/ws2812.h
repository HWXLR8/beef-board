#pragma once

#include <array>
#include <vector>

#include "rgb.h"

extern std::array<rgb_t, BAR_LEDS> bar_leds;
extern std::vector<rgb_t> tt_leds;

void ws2812_init();
void ws2812_show();

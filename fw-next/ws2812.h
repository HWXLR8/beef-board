#pragma once

#include <vector>

#include "rgb.h"

extern std::vector<rgb_t> bar_leds;
extern std::vector<rgb_t> tt_leds;

void ws2812_init();
void ws2812_show();
bool ready_to_show();

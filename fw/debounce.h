#pragma once

#include <stdint.h>

class DebounceState {
public:
	explicit DebounceState(uint8_t window);

	uint16_t debounce(uint16_t buttons);
private:
	uint16_t history[10]{};
	uint8_t window;
	uint16_t last_state{};
	uint32_t sample_time{};
	uint8_t current_index{};
};

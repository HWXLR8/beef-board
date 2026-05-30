#pragma once

#include <cstdint>

constexpr uint32_t analog_count = 2;
extern volatile uint8_t adc_buf[analog_count];

void adc_dma_init();

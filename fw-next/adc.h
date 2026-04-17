#pragma once

#include <cstdint>

constexpr uint32_t adc_dma_transfer_count = 2;
extern volatile uint8_t adc_buf[adc_dma_transfer_count];

void adc_dma_init();

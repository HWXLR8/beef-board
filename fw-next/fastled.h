#pragma once

#include <cstdint>

struct hsv_t;
struct rgb_t;

uint8_t quadwave8(uint8_t i);
void hsv2rgb_spectrum(const hsv_t &hsv, rgb_t &rgb);

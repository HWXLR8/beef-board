#pragma once

#include <cstdint>
#include <vector>

struct hsv_t;
struct rgb_t;

uint8_t quadwave8(uint8_t i);
void hsv2rgb_rainbow(const hsv_t &hsv, rgb_t &rgb);
void hsv2rgb_spectrum(const hsv_t &hsv, rgb_t &rgb);
void fill_rainbow(std::vector<rgb_t> &leds, uint8_t level, uint8_t initialHue, uint8_t deltaHue);
void fill_rainbow_circular(std::vector<rgb_t> &leds, uint8_t initialHue, uint8_t sat, uint8_t val,
                           bool reversed = false);

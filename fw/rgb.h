#pragma once

#include <stdint.h>

typedef struct {
  uint8_t r, g, b;
} rgb_light;

struct HSV {
  uint8_t h, s, v;
};

typedef struct {
  uint16_t buttons;
  rgb_light tt_lights;
  rgb_light bar_lights;
} hid_lights;

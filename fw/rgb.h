#pragma once

#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_light;

typedef struct {
    uint16_t buttons;
    rgb_light tt_lights;
} hid_lights;

#pragma once

#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_lights;

typedef struct {
    uint16_t buttons;
    rgb_lights tt_lights;
} hid_lights;

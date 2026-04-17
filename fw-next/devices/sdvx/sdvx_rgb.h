#pragma once

#include <cstdint>

namespace SDVX
{
    struct __attribute__((packed)) hid_lights_t
    {
        uint16_t buttons;
    };
}

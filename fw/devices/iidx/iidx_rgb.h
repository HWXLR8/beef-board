#pragma once

#include <stdint.h>

#include "../rgb.h"

namespace IIDX {
  struct hid_lights {
    uint16_t buttons;
    rgb_light tt_lights;
    rgb_light bar_lights;
  };
}

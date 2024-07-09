#pragma once

#include <stdint.h>

namespace SDVX {
  struct hid_lights {
    uint8_t report_id;
    uint16_t buttons;
  };
}

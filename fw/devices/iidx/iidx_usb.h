#pragma once

#include "../beef.h"
#include "../config.h"

namespace IIDX {
  void usb_init(config &config);
  void update_hid_interface(config &config);

  enum {
    BUTTON_TT_NEG = 1 << 11,
    BUTTON_TT_POS = 1 << 12,
    EFFECTORS_ALL = BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11
  };
}

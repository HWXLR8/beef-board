#pragma once

#include "../beef.h"
#include "../config.h"

namespace Iidx {
  void usb_init(config &config);

  enum {
    BUTTON_TT_NEG = 1 << 11,
    BUTTON_TT_POS = 1 << 12,
    EFFECTORS_ALL = BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11
  };
}

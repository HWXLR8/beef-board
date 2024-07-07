#pragma once

#include "../config.h"

namespace SDVX {
  void usb_init(config &config);
  void update_hid_interface(config &config);

  enum {
    BUTTON_ANALOG_X_NEG = BUTTON_7,
    BUTTON_ANALOG_X_POS = BUTTON_8,
    BUTTON_ANALOG_Y_NEG = BUTTON_10,
    BUTTON_ANALOG_Y_POS = BUTTON_11
  };
}

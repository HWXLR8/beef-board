#pragma once

namespace SDVX {
  void usb_desc_init();

  enum {
    // 7 physical + 2 -/+X + 2 -/+Y
    KEYBOARD_KEYS = 11
  };
}

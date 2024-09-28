#include "../beef.h"
#include "sdvx_combo.h"

namespace SDVX {
  enum {
    DISABLE_LED = BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4 | BUTTON_9
  };

  combo get_button_combo(uint16_t button_state) {
    switch (button_state) {
      case DISABLE_LED:
        return {
            .config_set = toggle_disable_leds
        };
      default:
        return {};
    }
  }
}

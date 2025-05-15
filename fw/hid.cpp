#include "hid.h"

bool reactive_led = true;
hid_state joystick_out_state = {
  .endpoint = JOYSTICK_OUT_EPADDR,
  .hid_expiry_timer = {}
};
hid_state lights_out_state = {
  .endpoint = LIGHTS_OUT_EPADDR,
  .hid_expiry_timer = {}
};

#include "hid.h"

// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
bool reactive_led = true;
bool rgb_standby = true;
timer hid_lights_expiry_timer;

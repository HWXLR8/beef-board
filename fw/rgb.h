#pragma once

#include <stdint.h>

struct rgb_light {
  uint8_t r, g, b;
};

struct HSV {
  uint8_t h, s, v;
};

struct hid_lights {
  uint16_t buttons;
  rgb_light tt_lights;
  rgb_light bar_lights;
};

enum class TurntableMode : uint8_t {
  STATIC,
  SPIN,
  SHIFT,
  RAINBOW_STATIC,
  RAINBOW_REACT,
  RAINBOW_SPIN,
  REACT,
  BREATHING,
  HID,
  DISABLE,
  COUNT
};

enum class BarMode : uint8_t {
  PLACEHOLDER1, // beat
  PLACEHOLDER2, // reactive p1
  PLACEHOLDER3, // audio spectrum
  PLACEHOLDER4, // reactive p2
  HID,
  DISABLE,
  COUNT
};

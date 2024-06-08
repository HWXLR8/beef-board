#pragma once

#include <stdint.h>

struct rgb_light {
  uint8_t r, g, b;
};

struct HSV {
  uint8_t h, s, v;
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
  KEY_SPECTRUM_P1,
  KEY_SPECTRUM_P2,
  PLACEHOLDER3, // audio spectrum
  HID,
  DISABLE,
  COUNT
};

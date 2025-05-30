#pragma once

#include <stdint.h>

struct HSV {
  uint8_t h, s, v;
};

enum class TurntableMode : uint8_t {
  Static,
  Spin,
  Shift,
  RainbowStatic,
  RainbowReact,
  RainbowSpin,
  React,
  Breathing,
  HID,
  Disable,
  Count
};

enum class BarMode : uint8_t {
  Placeholder1, // beat
  KeySpectrumP1,
  KeySpectrumP2,
  Placeholder3, // audio spectrum
  HID,
  TapeLedP1,
  TapeLedP2,
  Disable,
  Count
};

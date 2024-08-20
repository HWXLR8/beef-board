#pragma once

class Ticker {
public:
  explicit Ticker(uint8_t tick_duration);

  uint8_t get_ticks();
  void reset();
  void reset(uint8_t tick_duration);

private:
  uint8_t tick_duration;
  uint32_t last_tick_time{};
  bool first_tick = true;
};

#pragma once

class Ticker {
public:
  explicit Ticker(uint8_t tick_duration, uint16_t total_duration = 0);

  uint8_t get_ticks();
  void reset();
  void reset(uint8_t tick_duration, uint16_t total_duration = 0);

private:
  uint8_t tick_duration;
  uint32_t last_tick_time{};
  uint16_t tick_limit;
  uint16_t current_ticks{};

  bool first_tick = true;
};

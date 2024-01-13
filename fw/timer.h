#pragma once

#include <stdbool.h>
#include <stdint.h>

volatile uint32_t milliseconds;
typedef struct {
  bool armed;
  uint32_t time_to_expire;
} timer;

void timer_init(timer* self);
void timer_arm(timer* self, uint32_t milliseconds_from_now);
void timer_reset(timer* self);
bool timer_is_armed(timer* self);
bool timer_is_expired(timer* self);
int32_t timer_get_remaining_time(timer* self);
bool timer_check_if_expired_reset(timer* self);

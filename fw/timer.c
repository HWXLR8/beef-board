#include "timer.h"

volatile uint32_t milliseconds = 0;

void timer_init(timer* self) {
  self->armed = false;
  self->time_to_expire = 0;
}

void timer_arm(timer* self, uint32_t milliseconds_from_now) {
  uint32_t now = milliseconds;
  self->time_to_expire = now + milliseconds_from_now;
  self->armed = true;
}

void timer_reset(timer* self) {
  self->armed = false;
}

bool timer_is_armed(timer* self) {
  return self->armed;
}

bool timer_is_expired(timer* self) {
  if (!timer_is_armed(self)) {
    return false;
  }

  uint32_t now = milliseconds;
  int32_t diff = now - self->time_to_expire;
  return (diff > 0);
}

int32_t timer_get_remaining_time(timer* self) {
  // assumes that the timer is armed
  int32_t diff = self->time_to_expire - milliseconds;
  return diff;
}

bool timer_check_if_expired_reset(timer* self) {
  bool expired = timer_is_expired(self);
  if (expired) {
    timer_reset(self);
  }
  return expired;
}

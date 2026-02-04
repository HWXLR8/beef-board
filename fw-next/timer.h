#pragma once

#include "bsp/board_api.h"

struct timer_t
{
    bool armed = false;
    uint32_t expiry_time = 0;

    void arm(const uint32_t duration_ms)
    {
        this->expiry_time = board_millis() + duration_ms;
        this->armed = true;
    }

    void reset()
    {
        armed = false;
    }

    [[nodiscard]] bool is_expired() const
    {
        if (!armed)
        {
            return false;
        }

        int32_t diff = board_millis() - expiry_time;
        return diff > 0;
    }

    bool is_active()
    {
        return armed && !check_if_expired_and_reset();
    }

    bool check_if_expired_and_reset()
    {
        bool expired = is_expired();
        if (expired)
        {
            reset();
        }
        return expired;
    }
};

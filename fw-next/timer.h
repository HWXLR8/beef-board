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

    [[nodiscard]] bool is_expired(const bool reset_on_expiry = false)
    {
        if (!armed)
        {
            return false;
        }

        int32_t diff = board_millis() - expiry_time;
        const auto expired = diff >= 0;
        if (expired && reset_on_expiry)
            reset();
        return expired;
    }

    bool is_active()
    {
        return armed && !is_expired(true);
    }
};

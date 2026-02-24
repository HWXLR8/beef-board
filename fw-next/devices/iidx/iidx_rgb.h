#pragma once

#include "beef.h"
#include "iidx_combo.h"

namespace IIDX::RgbManager
{
    void update(int8_t tt1_report, const hid_lights_t &led_state_from_hid_report);
}

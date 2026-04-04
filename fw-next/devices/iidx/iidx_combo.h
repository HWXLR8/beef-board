#pragma once

#include "combo.h"

namespace IIDX
{
    extern timer_t combo_timer;

    combo_t get_button_combo();
    void on_combo_reset();
}

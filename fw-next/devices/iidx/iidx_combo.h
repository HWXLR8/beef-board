#pragma once

#include "combo.h"

namespace IIDX
{
    extern timer_t combo_timer;

    class ComboProcessor : public ::ComboProcessor
    {
    public:
        ComboProcessor() = default;

        combo_t get_button_combo() override;
        void on_reset() override;
    };
}

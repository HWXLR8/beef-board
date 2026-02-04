#pragma once
#include "combo.h"

namespace IIDX
{
    class ComboProcessor : public ::ComboProcessor
    {
    public:
        ComboProcessor() = default;

        combo_t get_button_combo() override;
    };
}

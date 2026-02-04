#pragma once
#include "beef.h"

namespace IIDX
{
    enum
    {
        BUTTON_TT_NEG = 1 << 11,
        BUTTON_TT_POS = 1 << 12,
        MAIN_BUTTONS_ALL = BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4 | BUTTON_5 | BUTTON_6 | BUTTON_7,
        EFFECTORS_ALL = BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11
    };
}

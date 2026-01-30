#include "fl/stl/stdint.h"

namespace fl::platform
{
    u32 micros()
    {
        return ::micros();
    }

    u32 millis()
    {
        return ::millis();
    }
}

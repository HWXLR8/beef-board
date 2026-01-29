#include "fl/stl/stdint.h"

namespace fl {
namespace platform {
    fl::u32 micros()
    {
        return ::micros();
    }
}
}
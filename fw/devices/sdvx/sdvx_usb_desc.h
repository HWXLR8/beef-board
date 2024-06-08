#pragma once

namespace Sdvx {
  uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                      const uint16_t wIndex,
                                      const void** const DescriptorAddress);
}

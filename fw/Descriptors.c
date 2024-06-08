#include "Descriptors.h"

const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"SEGV");
const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"BEEF BOARD");

uint16_t (*usb_desc_callback) (const uint16_t,
                               const uint16_t,
                               const void** const) = NULL;

// This function is called by the library when in device mode, and
// must be overridden (see library "USB Descriptors" documentation) by
// the application code so that the address and size of a requested
// descriptor can be given to the USB library. When the device
// receives a Get Descriptor request on the control endpoint, this
// function is called so that the descriptor details can be passed
// back and the appropriate descriptor sent back to the USB host.
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress) {
  return usb_desc_callback(wValue, wIndex, DescriptorAddress);
}

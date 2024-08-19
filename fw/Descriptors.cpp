#include "Descriptors.h"

const USB_Descriptor_HIDReport_Datatype_t* JoystickHIDReport;
uint16_t SizeOfJoystickHIDReport;
const USB_Descriptor_Device_t* DeviceDescriptor;
const USB_Descriptor_Configuration_t* ConfigurationDescriptor;
uint8_t LedStringCount;
const USB_Descriptor_String_t** LedStrings;

// Language descriptor structure. This descriptor, located in FLASH
// memory, is returned when the host requests the string descriptor
// with index 0 (the first index). It is actually an array of 16-bit
// integers, which indicate via the language ID table available at
// USB.org what languages the device supports for its string
// descriptors.
const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);

// Manufacturer descriptor string. This is a Unicode string containing
// the manufacturer's details in human readable form, and is read out
// upon request by the host when the appropriate string ID is
// requested, listed in the Device Descriptor.
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"SEGV");

// Product descriptor string. This is a Unicode string containing the
// product's details in human readable form, and is read out upon
// request by the host when the appropriate string ID is requested,
// listed in the Device Descriptor.
const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"BEEF BOARD");

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
  const uint8_t DescriptorType = (wValue >> 8);
  const uint8_t DescriptorNumber = (wValue & 0xFF);

  const void* Address = nullptr;
  uint16_t Size = NO_DESCRIPTOR;

  switch (DescriptorType) {
    case DTYPE_Device:
      Address = DeviceDescriptor;
      Size = sizeof(USB_Descriptor_Device_t);
      break;
    case DTYPE_Configuration:
      Address = ConfigurationDescriptor;
      Size = sizeof(USB_Descriptor_Configuration_t);
      break;
    case DTYPE_String:
      switch (DescriptorNumber) {
        case STRING_ID_Language:
          Address = &LanguageString;
          Size    = pgm_read_byte(&LanguageString.Header.Size);
          break;
        case STRING_ID_Manufacturer:
          Address = &ManufacturerString;
          Size    = pgm_read_byte(&ManufacturerString.Header.Size);
          break;
        case STRING_ID_Product:
          Address = &ProductString;
          Size    = pgm_read_byte(&ProductString.Header.Size);
          break;
        default:
          const int index = DescriptorNumber - LedStringBase - 1;
          if (0 <= index && index < LedStringCount) {
            Address = LedStrings[index];
            Size    = pgm_read_byte(&LedStrings[index]->Header.Size);
          }
          break;
      }
      break;
    case HID_DTYPE_HID:
      switch (wIndex)
      {
        case INTERFACE_ID_Joystick:
          Address = &ConfigurationDescriptor->HID_JoystickHID;
          Size    = sizeof(USB_HID_Descriptor_HID_t);
          break;
        case INTERFACE_ID_Keyboard:
          Address = &ConfigurationDescriptor->HID_KeyboardHID;
          Size    = sizeof(USB_HID_Descriptor_HID_t);
          break;
        case INTERFACE_ID_Mouse:
          Address = &ConfigurationDescriptor->HID_MouseHID;
          Size    = sizeof(USB_HID_Descriptor_HID_t);
          break;
        default:
          break;
      }
      break;
    case HID_DTYPE_Report:
      switch (wIndex)
      {
        case INTERFACE_ID_Joystick:
          Address = JoystickHIDReport;
          Size    = SizeOfJoystickHIDReport;
          break;
        case INTERFACE_ID_Keyboard:
          Address = &KeyboardHIDReport;
          Size    = sizeof(KeyboardHIDReport);
          break;
        case INTERFACE_ID_Mouse:
          Address = &MouseHIDReport;
          Size    = sizeof(MouseHIDReport);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  *DescriptorAddress = Address;
  return Size;
}

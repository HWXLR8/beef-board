#include "../Descriptors.h"
#include "sdvx_usb_desc.h"

// Hid class report descriptor. This is a special descriptor
// constructed with values from the USBIF Hid class specification to
// describe the reports and capabilities of the Hid device. This
// descriptor is parsed by the host and its contents used to determine
// what data (and in what encoding) the device will send, and what it
// may be sent back from the host. Refer to the Hid specification for
// more details on Hid report descriptors.

namespace SDVX {
  const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickReport[] = {
    HID_RI_USAGE_PAGE(8, 0x01),
    HID_RI_USAGE(8, 0x04),
    HID_RI_COLLECTION(8, 0x01),
      HID_RI_USAGE(8, 0x01),
      HID_RI_COLLECTION(8, 0x00),
        // Analog
        HID_RI_USAGE(8, 0x01),
        HID_RI_COLLECTION(8, 0x00),
          HID_RI_USAGE(8, 0x31), // Y
          HID_RI_USAGE(8, 0x30), // X
          HID_RI_LOGICAL_MINIMUM(16, 0),
          HID_RI_LOGICAL_MAXIMUM(16, 255),
          HID_RI_PHYSICAL_MINIMUM(16, -1),
          HID_RI_PHYSICAL_MAXIMUM(16, 1),
          HID_RI_REPORT_COUNT(8, 2),
          HID_RI_REPORT_SIZE(8, 8),
          HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
        HID_RI_END_COLLECTION(0),

        // Buttons
        // 7 physical (for some reason START is bound to B9 in EAC)
        HID_BUTTONS(9),
        HID_PADDING_INPUT(7),

        // Button lighting
        HID_BUTTON_LIGHT(1),
        HID_BUTTON_LIGHT(2),
        HID_BUTTON_LIGHT(3),
        HID_BUTTON_LIGHT(4),
        HID_BUTTON_LIGHT(5),
        HID_BUTTON_LIGHT(6),
        HID_PADDING_OUTPUT(2),
        HID_BUTTON_LIGHT(7),
        HID_PADDING_OUTPUT(7),
      HID_RI_END_COLLECTION(0),
    HID_RI_END_COLLECTION(0),
  };

  const USB_Descriptor_HIDReport_Datatype_t PROGMEM KeyboardReport[] = {
    HID_KEYBOARD_DESCRIPTOR(KEYBOARD_KEYS)
  };

  const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
    // official Konami SOUND VOLTEX NEMSYS controller VID/PID
    DEVICE_DESCRIPTOR(0x1CCF, 0x101C)
  };

  const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
    CONFIGURATION_DESCRIPTOR(JoystickReport, KeyboardReport)
  };

  enum {
    LedStringCount = 7
  };
  const USB_Descriptor_String_t PROGMEM led_name1 = USB_STRING_DESCRIPTOR(L"BT-A");
  const USB_Descriptor_String_t PROGMEM led_name2 = USB_STRING_DESCRIPTOR(L"BT-B");
  const USB_Descriptor_String_t PROGMEM led_name3 = USB_STRING_DESCRIPTOR(L"BT-C");
  const USB_Descriptor_String_t PROGMEM led_name4 = USB_STRING_DESCRIPTOR(L"BT-D");
  const USB_Descriptor_String_t PROGMEM led_name5 = USB_STRING_DESCRIPTOR(L"FX-L");
  const USB_Descriptor_String_t PROGMEM led_name6 = USB_STRING_DESCRIPTOR(L"FX-R");
  const USB_Descriptor_String_t PROGMEM led_name7 = USB_STRING_DESCRIPTOR(L"Start");
  const USB_Descriptor_String_t* led_names[LedStringCount] = {
    &led_name1,
    &led_name2,
    &led_name3,
    &led_name4,
    &led_name5,
    &led_name6,
    &led_name7
  };

  void usb_desc_init() {
    ::JoystickReport = JoystickReport;
    ::SizeOfJoystickReport = sizeof(JoystickReport);
    ::KeyboardReport = KeyboardReport;
    ::SizeOfKeyboardReport = sizeof(KeyboardReport);
    ::DeviceDescriptor = &DeviceDescriptor;
    ::ConfigurationDescriptor = &ConfigurationDescriptor;
    ::LedStringCount = LedStringCount;
    ::LedStrings = led_names;
  }
}

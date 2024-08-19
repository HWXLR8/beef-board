#include "../Descriptors.h"
#include "sdvx_usb_desc.h"

namespace SDVX {
  const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickHIDReport[] = {
    HID_RI_USAGE_PAGE(8, 0x01),
    HID_RI_USAGE(8, 0x04),
    HID_RI_COLLECTION(8, 0x01),
      // Analog
      HID_RI_USAGE(8, 0x01),
      HID_RI_COLLECTION(8, 0x02),
        HID_RI_USAGE(8, 0x30), // X
        HID_RI_USAGE(8, 0x31), // Y
        HID_RI_LOGICAL_MINIMUM(16, 0),
        HID_RI_LOGICAL_MAXIMUM(16, 255),
        HID_RI_PHYSICAL_MINIMUM(8, -1),
        HID_RI_PHYSICAL_MAXIMUM(8, 1),
        HID_RI_REPORT_COUNT(8, 0x02),
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
      HID_RI_END_COLLECTION(0),

      // Buttons
      // 7 physical (for some reason START is bound to B9 in EAC)
      HID_BUTTONS(9),

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
    HID_RI_END_COLLECTION(0)
  };

  // official Konami SOUND VOLTEX NEMSYS controller VID/PID
  const auto PROGMEM DeviceDescriptor = generate_device_descriptor(0x1CCF, 0x101C);

  const auto PROGMEM ConfigurationDescriptor = generate_configuration_descriptor(sizeof(JoystickHIDReport));

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
    ::JoystickHIDReport = JoystickHIDReport;
    ::SizeOfJoystickHIDReport = sizeof(JoystickHIDReport);
    ::DeviceDescriptor = &DeviceDescriptor;
    ::ConfigurationDescriptor = &ConfigurationDescriptor;
    ::LedStringCount = LedStringCount;
    ::LedStrings = led_names;
  }
}

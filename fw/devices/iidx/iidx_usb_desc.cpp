#include "../Descriptors.h"
#include "iidx_usb_desc.h"

namespace IIDX {
  const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickHIDReport[] = {
    HID_RI_USAGE_PAGE(8, 0x01),
    HID_RI_USAGE(8, 0x04),
    HID_RI_COLLECTION(8, 0x01),
      // Analog
      HID_RI_USAGE(8, 0x01),
      HID_RI_COLLECTION(8, 0x02),
        HID_RI_USAGE(8, 0x30), // X
        HID_RI_LOGICAL_MINIMUM(16, 0),
        HID_RI_LOGICAL_MAXIMUM(16, 255),
        HID_RI_PHYSICAL_MINIMUM(8, -1),
        HID_RI_PHYSICAL_MAXIMUM(8, 1),
        HID_RI_REPORT_COUNT(8, 0x01),
        HID_RI_REPORT_SIZE(8, 0x08),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
      HID_RI_END_COLLECTION(0),

      // Buttons
      // 11 physical (7 + 1 padding (Infinitas) + 4 extra) + digital TT (-/+)
      HID_BUTTONS(14),

      // Button lighting
      HID_BUTTON_LIGHT(1),
      HID_BUTTON_LIGHT(2),
      HID_BUTTON_LIGHT(3),
      HID_BUTTON_LIGHT(4),
      HID_BUTTON_LIGHT(5),
      HID_BUTTON_LIGHT(6),
      HID_BUTTON_LIGHT(7),
      HID_BUTTON_LIGHT(8),
      HID_BUTTON_LIGHT(9),
      HID_BUTTON_LIGHT(10),
      HID_BUTTON_LIGHT(11),
      HID_PADDING_OUTPUT(5),

      // TT WS2812
      HID_RGB(12),

      // Bar WS2812
      HID_RGB(15),
    HID_RI_END_COLLECTION(0)
  };

  // official Konami infinitas controller VID/PID
  const auto PROGMEM DeviceDescriptor = generate_device_descriptor(0x1CCF, 0x8048);

  const auto PROGMEM ConfigurationDescriptor = generate_configuration_descriptor(sizeof(JoystickHIDReport));

  enum {
    LedStringCount = 17
  };
  const USB_Descriptor_String_t PROGMEM led_name1 = USB_STRING_DESCRIPTOR(L"Button 1");
  const USB_Descriptor_String_t PROGMEM led_name2 = USB_STRING_DESCRIPTOR(L"Button 2");
  const USB_Descriptor_String_t PROGMEM led_name3 = USB_STRING_DESCRIPTOR(L"Button 3");
  const USB_Descriptor_String_t PROGMEM led_name4 = USB_STRING_DESCRIPTOR(L"Button 4");
  const USB_Descriptor_String_t PROGMEM led_name5 = USB_STRING_DESCRIPTOR(L"Button 5");
  const USB_Descriptor_String_t PROGMEM led_name6 = USB_STRING_DESCRIPTOR(L"Button 6");
  const USB_Descriptor_String_t PROGMEM led_name7 = USB_STRING_DESCRIPTOR(L"Button 7");
  const USB_Descriptor_String_t PROGMEM led_name8 = USB_STRING_DESCRIPTOR(L"Button 8");
  const USB_Descriptor_String_t PROGMEM led_name9 = USB_STRING_DESCRIPTOR(L"Button 9");
  const USB_Descriptor_String_t PROGMEM led_name10 = USB_STRING_DESCRIPTOR(L"Button 10");
  const USB_Descriptor_String_t PROGMEM led_name11 = USB_STRING_DESCRIPTOR(L"Button 11");
  const USB_Descriptor_String_t PROGMEM led_name12 = USB_STRING_DESCRIPTOR(L"Turntable R");
  const USB_Descriptor_String_t PROGMEM led_name13 = USB_STRING_DESCRIPTOR(L"Turntable G");
  const USB_Descriptor_String_t PROGMEM led_name14 = USB_STRING_DESCRIPTOR(L"Turntable B");
  const USB_Descriptor_String_t PROGMEM led_name15 = USB_STRING_DESCRIPTOR(L"Centre Bar R");
  const USB_Descriptor_String_t PROGMEM led_name16 = USB_STRING_DESCRIPTOR(L"Centre Bar G");
  const USB_Descriptor_String_t PROGMEM led_name17 = USB_STRING_DESCRIPTOR(L"Centre Bar B");
  const USB_Descriptor_String_t* led_names[LedStringCount] = {
    &led_name1,
    &led_name2,
    &led_name3,
    &led_name4,
    &led_name5,
    &led_name6,
    &led_name7,
    &led_name8,
    &led_name9,
    &led_name10,
    &led_name11,
    &led_name12,
    &led_name13,
    &led_name14,
    &led_name15,
    &led_name16,
    &led_name17
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

#include "../Descriptors.h"
#include "iidx_usb_desc.h"

// Hid class report descriptor. This is a special descriptor
// constructed with values from the USBIF Hid class specification to
// describe the reports and capabilities of the Hid device. This
// descriptor is parsed by the host and its contents used to determine
// what data (and in what encoding) the device will send, and what it
// may be sent back from the host. Refer to the Hid specification for
// more details on Hid report descriptors.

namespace IIDX {
  const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickReport[] = {
    HID_RI_USAGE_PAGE(8, 0x01),
    HID_RI_USAGE(8, 0x04),
    HID_RI_COLLECTION(8, 0x01),
      HID_RI_USAGE(8, 0x01),
      HID_RI_COLLECTION(8, 0x00),
        // Analog
        HID_RI_USAGE(8, 0x01),
        HID_RI_COLLECTION(8, 0x00),
          HID_RI_USAGE(8, 0x30), // X
          HID_RI_LOGICAL_MINIMUM(16, 0),
          HID_RI_LOGICAL_MAXIMUM(16, 255),
          HID_RI_PHYSICAL_MINIMUM(16, -1),
          HID_RI_PHYSICAL_MAXIMUM(16, 1),
          HID_RI_REPORT_COUNT(8, 1),
          HID_RI_REPORT_SIZE(8, 8),
          HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
        HID_RI_END_COLLECTION(0),

        // Buttons
        // 11 physical (7 + 1 padding (Infinitas) + 4 extra) + digital TT (-/+)
        HID_BUTTONS(14),
        HID_PADDING_INPUT(2),

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
      HID_RI_END_COLLECTION(0),
    HID_RI_END_COLLECTION(0),
  };

  const USB_Descriptor_HIDReport_Datatype_t PROGMEM KeyboardReport[] = {
    HID_KEYBOARD_DESCRIPTOR(KEYBOARD_KEYS)
  };

  const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
    // official Konami infinitas controller VID/PID
    DEVICE_DESCRIPTOR(0x1CCF, 0x8048)
  };

  const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
    CONFIGURATION_DESCRIPTOR(JoystickReport, KeyboardReport)
  };

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

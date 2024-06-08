#pragma once

#include <avr/pgmspace.h>
#include <LUFA/Drivers/USB/USB.h>

#define LedStringBase  0x10

#define HID_RI_STRING_INDEX(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x70, DataBits, __VA_ARGS__)

#define HID_BUTTONS(Number) \
HID_RI_USAGE_PAGE(8, 0x09), \
HID_RI_USAGE_MINIMUM(8, 0x01), \
HID_RI_USAGE_MAXIMUM(8, Number), \
HID_RI_LOGICAL_MINIMUM(8, 0x00), \
HID_RI_LOGICAL_MAXIMUM(8, 0x01), \
HID_RI_REPORT_SIZE(8, 0x01), \
HID_RI_REPORT_COUNT(8, Number), \
HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE)

#define HID_BUTTON_LIGHT(Number) \
HID_RI_USAGE_PAGE(8, 0x0A), \
HID_RI_USAGE(8, Number), \
HID_RI_COLLECTION(8, 0x02), \
  HID_RI_USAGE_PAGE(8, 0x09), \
  HID_RI_USAGE(8, Number), \
  HID_RI_STRING_INDEX(8, LedStringBase+Number), \
  HID_RI_REPORT_SIZE(8, 0x01), \
  HID_RI_REPORT_COUNT(8, 0x01), \
  HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE), \
HID_RI_END_COLLECTION(0)

#define HID_RGB(Number) \
HID_RI_LOGICAL_MINIMUM(8, 0), \
HID_RI_LOGICAL_MAXIMUM(8, 255), \
HID_RI_USAGE_PAGE(8, 0x0A), \
HID_RI_USAGE(8, 1), \
HID_RI_COLLECTION(8, 0x02), \
  HID_RI_USAGE_PAGE(8, 0x08), \
  HID_RI_USAGE(8, 0x4B), \
  HID_RI_STRING_INDEX(8, LedStringBase+Number), \
  HID_RI_REPORT_SIZE(8, 0x08), \
  HID_RI_REPORT_COUNT(8, 0x01), \
  HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE), \
HID_RI_END_COLLECTION(0), \
HID_RI_USAGE(8, 2), \
HID_RI_COLLECTION(8, 0x02), \
  HID_RI_USAGE_PAGE(8, 0x08), \
  HID_RI_USAGE(8, 0x4B), \
  HID_RI_STRING_INDEX(8, LedStringBase+Number+1), \
  HID_RI_REPORT_SIZE(8, 0x08), \
  HID_RI_REPORT_COUNT(8, 0x01), \
  HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE), \
HID_RI_END_COLLECTION(0), \
HID_RI_USAGE(8, 3), \
HID_RI_COLLECTION(8, 0x02), \
  HID_RI_USAGE_PAGE(8, 0x08), \
  HID_RI_USAGE(8, 0x4B), \
  HID_RI_STRING_INDEX(8, LedStringBase+Number+2), \
  HID_RI_REPORT_SIZE(8, 0x08), \
  HID_RI_REPORT_COUNT(8, 0x01), \
  HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE), \
HID_RI_END_COLLECTION(0)

#define HID_PADDING_INPUT(Number) \
HID_RI_REPORT_SIZE(8, Number), \
HID_RI_REPORT_COUNT(8, 0x01), \
HID_RI_INPUT(8, HID_IOF_CONSTANT)

#define HID_PADDING_OUTPUT(Number) \
HID_RI_REPORT_SIZE(8, Number), \
HID_RI_REPORT_COUNT(8, 0x01), \
HID_RI_OUTPUT(8, HID_IOF_CONSTANT)

// Language descriptor structure. This descriptor, located in FLASH
// memory, is returned when the host requests the string descriptor
// with index 0 (the first index). It is actually an array of 16-bit
// integers, which indicate via the language ID table available at
// USB.org what languages the device supports for its string
// descriptors.
extern const USB_Descriptor_String_t PROGMEM LanguageString;

// Manufacturer descriptor string. This is a Unicode string containing
// the manufacturer's details in human readable form, and is read out
// upon request by the host when the appropriate string ID is
// requested, listed in the Device Descriptor.
extern const USB_Descriptor_String_t PROGMEM ManufacturerString;

// Product descriptor string. This is a Unicode string containing the
// product's details in human readable form, and is read out upon
// request by the host when the appropriate string ID is requested,
// listed in the Device Descriptor.
extern const USB_Descriptor_String_t PROGMEM ProductString;

// Type define for the device configuration descriptor structure. This
// must be defined in the application code, as the configuration
// descriptor contains several sub-descriptors which vary between
// devices, and which describe the device's usage to the host.

typedef struct {
  USB_Descriptor_Configuration_Header_t Config;
  USB_Descriptor_Interface_t HID_Interface;
  USB_HID_Descriptor_HID_t HID_JoystickHID;
  USB_Descriptor_Endpoint_t HID_ReportINEndpoint;
  USB_Descriptor_Endpoint_t HID_ReportOUTEndpoint;
} USB_Descriptor_Configuration_t;

// Enum for the device interface descriptor IDs within the
// device. Each interface descriptor should have a unique ID index
// associated with it, which can be used to refer to the interface
// from other descriptors.
enum InterfaceDescriptors_t {
  INTERFACE_ID_Joystick = 0, /**< Joystick interface desciptor ID */
};

// Enum for the device string descriptor IDs within the device. Each
// string descriptor should have a unique ID index associated with it,
// which can be used to refer to the string from other descriptors.
enum StringDescriptors_t {
  STRING_ID_Language = 0, // Supported Languages string descriptor ID (must be zero)
  STRING_ID_Manufacturer = 1, // Manufacturer string ID
  STRING_ID_Product = 2, // Product string ID
};

// Endpoint address of the Joystick HID reporting IN endpoint
#define JOYSTICK_IN_EPADDR (ENDPOINT_DIR_IN | 1)

// Endpoint address of the Joystick HID reporting OUT endpoint
#define JOYSTICK_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)

// Size of Joystick HID reporting IN endpoint in bytes
#define JOYSTICK_EPSIZE 8

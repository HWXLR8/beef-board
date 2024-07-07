#pragma once

#include <avr/pgmspace.h>
#include <LUFA/Drivers/USB/USB.h>

#define LedStringBase 0x10

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

#define HID_KEYBOARD_DESCRIPTOR(MaxKeys) \
HID_RI_USAGE_PAGE(8, 0x01), \
HID_RI_USAGE(8, 0x06), \
HID_RI_COLLECTION(8, 0x01), \
  HID_RI_USAGE_PAGE(8, 0x07), \
  HID_RI_USAGE_MINIMUM(8, 0x00), \
  HID_RI_USAGE_MAXIMUM(8, 0xFF), \
  HID_RI_LOGICAL_MINIMUM(8, 0x00), \
  HID_RI_LOGICAL_MAXIMUM(16, 0xFF), \
  HID_RI_REPORT_COUNT(8, MaxKeys), \
  HID_RI_REPORT_SIZE(8, 0x08), \
  HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE), \
HID_RI_END_COLLECTION(0)

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

  USB_Descriptor_Interface_t HID_JoystickInterface;
  USB_HID_Descriptor_HID_t HID_JoystickHID;
  USB_Descriptor_Endpoint_t HID_JoystickReportINEndpoint;
  USB_Descriptor_Endpoint_t HID_JoystickReportOUTEndpoint;

  USB_Descriptor_Interface_t HID_KeyboardInterface;
  USB_HID_Descriptor_HID_t HID_KeyboardHID;
  USB_Descriptor_Endpoint_t HID_KeyboardReportINEndpoint;
} USB_Descriptor_Configuration_t;

extern const USB_Descriptor_HIDReport_Datatype_t* JoystickReport;
extern uint16_t SizeOfJoystickReport;
extern const USB_Descriptor_HIDReport_Datatype_t* KeyboardReport;
extern uint16_t SizeOfKeyboardReport;
extern const USB_Descriptor_Device_t* DeviceDescriptor;
extern const USB_Descriptor_Configuration_t* ConfigurationDescriptor;
extern uint8_t LedStringCount;
extern const USB_Descriptor_String_t** LedStrings;

// Enum for the device interface descriptor IDs within the
// device. Each interface descriptor should have a unique ID index
// associated with it, which can be used to refer to the interface
// from other descriptors.
enum InterfaceDescriptors_t {
  INTERFACE_ID_Joystick = 0, /**< Joystick interface descriptor ID */
  INTERFACE_ID_Keyboard = 1, /**< Keyboard interface descriptor ID */
};

// Enum for the device string descriptor IDs within the device. Each
// string descriptor should have a unique ID index associated with it,
// which can be used to refer to the string from other descriptors.
enum StringDescriptors_t {
  STRING_ID_Language = 0, // Supported Languages string descriptor ID (must be zero)
  STRING_ID_Manufacturer = 1, // Manufacturer string ID
  STRING_ID_Product = 2, // Product string ID
};

// Device descriptor structure. This descriptor, located in FLASH
// memory, describes the overall device characteristics, including the
// supported USB version, control endpoint size and the number of
// device configurations. The descriptor is read out by the USB host
// when the enumeration process begins.
#define DEVICE_DESCRIPTOR(VID, PID) \
.Header = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device}, \
.USBSpecification = VERSION_BCD(1,1,0), \
.Class = USB_CSCP_NoDeviceClass, \
.SubClass = USB_CSCP_NoDeviceSubclass, \
.Protocol = USB_CSCP_NoDeviceProtocol, \
.Endpoint0Size = FIXED_CONTROL_ENDPOINT_SIZE, \
.VendorID = VID, \
.ProductID = PID, \
.ReleaseNumber = VERSION_BCD(0,0,1), \
.ManufacturerStrIndex = STRING_ID_Manufacturer, \
.ProductStrIndex = STRING_ID_Product, \
.SerialNumStrIndex = NO_DESCRIPTOR, \
.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS

// Endpoint address of the Joystick Hid reporting IN endpoint
#define JOYSTICK_IN_EPADDR (ENDPOINT_DIR_IN | 1)

// Endpoint address of the Joystick Hid reporting OUT endpoint
#define JOYSTICK_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)

// Endpoint address of the Keyboard Hid reporting IN endpoint.
#define KEYBOARD_IN_EPADDR (ENDPOINT_DIR_IN | 3)

// Size of Joystick Hid reporting IN endpoint in bytes
#define HID_EPSIZE 8

// Configuration descriptor structure. This descriptor, located in
// FLASH memory, describes the usage of the device in one of its
// supported configurations, including information about any device
// interfaces and endpoints. The descriptor is read out by the USB
// host during the enumeration process when selecting a configuration
// so that the host may correctly communicate with the USB device.
#define CONFIGURATION_DESCRIPTOR(JoystickReport, KeyboardReport) \
.Config = { \
  .Header = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration}, \
  .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t), \
  .TotalInterfaces = 2, \
  .ConfigurationNumber = 1, \
  .ConfigurationStrIndex = NO_DESCRIPTOR, \
  .ConfigAttributes = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED), \
  .MaxPowerConsumption = USB_CONFIG_POWER_MA(500) \
}, \
.HID_JoystickInterface = { \
  .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface}, \
  .InterfaceNumber = INTERFACE_ID_Joystick, \
  .AlternateSetting = 0x00, \
  .TotalEndpoints = 2, \
  .Class = HID_CSCP_HIDClass, \
  .SubClass = HID_CSCP_NonBootSubclass, \
  .Protocol = HID_CSCP_NonBootProtocol, \
  .InterfaceStrIndex = NO_DESCRIPTOR \
}, \
.HID_JoystickHID = { \
  .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID}, \
  .HIDSpec = VERSION_BCD(1,1,1), \
  .CountryCode = 0x00, \
  .TotalReportDescriptors = 1, \
  .HIDReportType = HID_DTYPE_Report, \
  .HIDReportLength = sizeof(JoystickReport) \
}, \
.HID_JoystickReportINEndpoint = { \
  .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint}, \
  .EndpointAddress = JOYSTICK_IN_EPADDR, \
  .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA), \
  .EndpointSize = HID_EPSIZE, \
  .PollingIntervalMS = 0x01 \
}, \
.HID_JoystickReportOUTEndpoint = { \
  .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint}, \
  .EndpointAddress = JOYSTICK_OUT_EPADDR, \
  .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA), \
  .EndpointSize = HID_EPSIZE, \
  .PollingIntervalMS = 0x01 \
}, \
.HID_KeyboardInterface = { \
  .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface}, \
  .InterfaceNumber = INTERFACE_ID_Keyboard, \
  .AlternateSetting = 0x00, \
  .TotalEndpoints = 1, \
  .Class = HID_CSCP_HIDClass, \
  .SubClass = HID_CSCP_NonBootSubclass, \
  .Protocol = HID_CSCP_NonBootProtocol, \
  .InterfaceStrIndex = NO_DESCRIPTOR \
}, \
.HID_KeyboardHID = { \
  .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID}, \
  .HIDSpec = VERSION_BCD(1,1,1), \
  .CountryCode = 0x00, \
  .TotalReportDescriptors = 1, \
  .HIDReportType = HID_DTYPE_Report, \
  .HIDReportLength = sizeof(KeyboardReport) \
}, \
.HID_KeyboardReportINEndpoint = { \
  .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint}, \
  .EndpointAddress = KEYBOARD_IN_EPADDR, \
  .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA), \
  .EndpointSize = HID_EPSIZE, \
  .PollingIntervalMS = 0x01 \
}

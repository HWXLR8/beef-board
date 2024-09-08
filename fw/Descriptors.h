#pragma once

#include <avr/pgmspace.h>
#include <LUFA/Drivers/USB/USB.h>

#include "config.h"

#define LedStringBase 0x10

#define HID_RI_STRING_INDEX(DataBits, ...) _HID_RI_ENTRY(HID_RI_TYPE_LOCAL, 0x70, DataBits, __VA_ARGS__)

#define HID_PADDING_INPUT(Number) \
HID_RI_REPORT_SIZE(8, Number), \
HID_RI_REPORT_COUNT(8, 0x01), \
HID_RI_INPUT(8, HID_IOF_CONSTANT)

#define HID_PADDING_OUTPUT(Number) \
HID_RI_REPORT_SIZE(8, Number), \
HID_RI_REPORT_COUNT(8, 0x01), \
HID_RI_OUTPUT(8, HID_IOF_CONSTANT)

#define HID_BUTTONS(Buttons) \
HID_RI_USAGE_PAGE(8, 0x09), \
HID_RI_USAGE_MINIMUM(8, 0x01), \
HID_RI_USAGE_MAXIMUM(8, Buttons), \
HID_RI_LOGICAL_MINIMUM(8, 0x00), \
HID_RI_LOGICAL_MAXIMUM(8, 0x01), \
HID_RI_REPORT_SIZE(8, 0x01), \
HID_RI_REPORT_COUNT(8, Buttons), \
HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE), \
HID_PADDING_INPUT((Buttons % 8) ? (8 - (Buttons % 8)) : 0)

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
HID_RI_LOGICAL_MINIMUM(8, 0x00), \
HID_RI_LOGICAL_MAXIMUM(8, 0xFF), \
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

// Type define for the device configuration descriptor structure. This
// must be defined in the application code, as the configuration
// descriptor contains several sub-descriptors which vary between
// devices, and which describe the device's usage to the host.
//
// Each interface can only support one IN and OUT endpoint,
// so for KBM support we need to use multiple interfaces as annoying as it is.
// Technically keyboard data could be sent in the joystick IN endpoint, but might as well do things "cleanly".
typedef struct {
  USB_Descriptor_Configuration_Header_t Config;

  USB_Descriptor_Interface_t HID_JoystickInterface;
  USB_HID_Descriptor_HID_t HID_JoystickHID;
  USB_Descriptor_Endpoint_t HID_JoystickReportINEndpoint;
  USB_Descriptor_Endpoint_t HID_JoystickReportOUTEndpoint;

  USB_Descriptor_Interface_t HID_KeyboardInterface;
  USB_HID_Descriptor_HID_t HID_KeyboardHID;
  USB_Descriptor_Endpoint_t HID_KeyboardReportINEndpoint;

  USB_Descriptor_Interface_t HID_MouseInterface;
  USB_HID_Descriptor_HID_t HID_MouseHID;
  USB_Descriptor_Endpoint_t HID_MouseReportINEndpoint;

  USB_Descriptor_Interface_t HID_ConfigInterface;
  USB_HID_Descriptor_HID_t HID_ConfigHID;
} USB_Descriptor_Configuration_t;

// HID class report descriptor. This is a special descriptor
// constructed with values from the USBIF HID class specification to
// describe the reports and capabilities of the HID device. This
// descriptor is parsed by the host and its contents used to determine
// what data (and in what encoding) the device will send, and what it
// may be sent back from the host. Refer to the HID specification for
// more details on HID report descriptors.
extern const USB_Descriptor_HIDReport_Datatype_t* JoystickHIDReport;
extern uint16_t SizeOfJoystickHIDReport;
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
  INTERFACE_ID_Mouse    = 2, /**< Mouse interface descriptor ID */
  INTERFACE_ID_Config   = 3, /**< Config interface descriptor ID */
};

// Enum for the device string descriptor IDs within the device. Each
// string descriptor should have a unique ID index associated with it,
// which can be used to refer to the string from other descriptors.
enum StringDescriptors_t {
  STRING_ID_Language = 0, // Supported Languages string descriptor ID (must be zero)
  STRING_ID_Manufacturer = 1, // Manufacturer string ID
  STRING_ID_Product = 2, // Product string ID
};

enum {
  KEYBOARD_KEYS = 13 // Max used keycodes
};

enum {
  HID_REPORTID_Config = 0x01,
  HID_REPORTID_Command = 0x02,
  HID_REPORTID_FirmwareVersion = 0x03
};

const USB_Descriptor_HIDReport_Datatype_t PROGMEM KeyboardHIDReport[] = {
  HID_RI_USAGE_PAGE(8, 0x01),
  HID_RI_USAGE(8, 0x06),
  HID_RI_COLLECTION(8, 0x01),
    HID_RI_USAGE_PAGE(8, 0x07),
    HID_RI_USAGE_MINIMUM(8, 0x00),
    HID_RI_USAGE_MAXIMUM(16, 0xFF),
    HID_RI_LOGICAL_MINIMUM(8, 0x00),
    HID_RI_LOGICAL_MAXIMUM(16, 0xFF),
    HID_RI_REPORT_COUNT(8, KEYBOARD_KEYS),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
  HID_RI_END_COLLECTION(0)
};

const USB_Descriptor_HIDReport_Datatype_t PROGMEM MouseHIDReport[] {
  HID_RI_USAGE_PAGE(8, 0x01),
  HID_RI_USAGE(8, 0x02),
  HID_RI_COLLECTION(8, 0x01),
    HID_RI_USAGE(8, 0x01),
    HID_RI_COLLECTION(8, 0x02),
      HID_RI_USAGE(8, 0x31),
      HID_RI_USAGE(8, 0x30),
      HID_RI_LOGICAL_MINIMUM(8, -1),
      HID_RI_LOGICAL_MAXIMUM(8, 1),
      HID_RI_PHYSICAL_MINIMUM(8, -1),
      HID_RI_PHYSICAL_MAXIMUM(8, 1),
      HID_RI_REPORT_COUNT(8, 0x02),
      HID_RI_REPORT_SIZE(8, 0x08),
      HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),
    HID_RI_END_COLLECTION(0),
  HID_RI_END_COLLECTION(0)
};

const USB_Descriptor_HIDReport_Datatype_t PROGMEM ConfigHIDReport[] {
  HID_RI_USAGE_PAGE(16, 0xFF00),
  HID_RI_USAGE(8, 0x01),
  HID_RI_LOGICAL_MINIMUM(8, 0x00),
  HID_RI_LOGICAL_MAXIMUM(16, 0xFF),
  HID_RI_COLLECTION(8, 0x01),
    HID_RI_REPORT_ID(8, HID_REPORTID_Config),
    HID_RI_USAGE(8, 0x01),
    HID_RI_REPORT_COUNT(8, sizeof(config)),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_FEATURE(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

    HID_RI_REPORT_ID(8, HID_REPORTID_Command),
    HID_RI_USAGE(8, 0x02),
    HID_RI_REPORT_COUNT(8, 0x01),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_FEATURE(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

    HID_RI_REPORT_ID(8, HID_REPORTID_FirmwareVersion),
    HID_RI_USAGE(8, 0x03),
    HID_RI_REPORT_COUNT(8, sizeof(uint32_t)),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_FEATURE(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
  HID_RI_END_COLLECTION(0)
};

// Device descriptor structure. This descriptor, located in FLASH
// memory, describes the overall device characteristics, including the
// supported USB version, control endpoint size and the number of
// device configurations. The descriptor is read out by the USB host
// when the enumeration process begins.
constexpr USB_Descriptor_Device_t generate_device_descriptor(uint16_t vid, uint16_t pid) {
  return USB_Descriptor_Device_t{
    .Header = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
    .USBSpecification = VERSION_BCD(1,1,0),
    .Class = USB_CSCP_NoDeviceClass,
    .SubClass = USB_CSCP_NoDeviceSubclass,
    .Protocol = USB_CSCP_NoDeviceProtocol,
    .Endpoint0Size = FIXED_CONTROL_ENDPOINT_SIZE,
    .VendorID = vid,
    .ProductID = pid,
    .ReleaseNumber = VERSION_BCD(0,0,1),
    .ManufacturerStrIndex = STRING_ID_Manufacturer,
    .ProductStrIndex = STRING_ID_Product,
    .SerialNumStrIndex = NO_DESCRIPTOR,
    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
  };
}

// Endpoint address of the Joystick HID reporting IN endpoint
#define JOYSTICK_IN_EPADDR (ENDPOINT_DIR_IN | 1)

// Endpoint address of the Joystick HID reporting OUT endpoint
#define JOYSTICK_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)

// Endpoint address of the Keyboard HID reporting IN endpoint
#define KEYBOARD_IN_EPADDR (ENDPOINT_DIR_IN | 3)

// Endpoint address of the Mouse HID reporting IN endpoint
#define MOUSE_IN_EPADDR (ENDPOINT_DIR_IN | 4)

// Size of Hid reporting IN/OUT endpoint in bytes
#define HID_EPSIZE FIXED_CONTROL_ENDPOINT_SIZE

// Configuration descriptor structure. This descriptor, located in
// FLASH memory, describes the usage of the device in one of its
// supported configurations, including information about any device
// interfaces and endpoints. The descriptor is read out by the USB
// host during the enumeration process when selecting a configuration
// so that the host may correctly communicate with the USB device.
constexpr USB_Descriptor_Configuration_t generate_configuration_descriptor(uint16_t joystick_hid_report_size) {
  return USB_Descriptor_Configuration_t{
    .Config = {
      .Header = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},
      .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
      .TotalInterfaces = 4,
      .ConfigurationNumber = 1,
      .ConfigurationStrIndex = NO_DESCRIPTOR,
      .ConfigAttributes = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),
      .MaxPowerConsumption = USB_CONFIG_POWER_MA(500)
    },
    .HID_JoystickInterface = {
      .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber = INTERFACE_ID_Joystick,
      .AlternateSetting = 0x00,
      .TotalEndpoints = 2,
      .Class = HID_CSCP_HIDClass,
      .SubClass = HID_CSCP_NonBootSubclass,
      .Protocol = HID_CSCP_NonBootProtocol,
      .InterfaceStrIndex = NO_DESCRIPTOR
    },
    .HID_JoystickHID = {
      .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
      .HIDSpec = VERSION_BCD(1,1,1),
      .CountryCode = 0x00,
      .TotalReportDescriptors = 1,
      .HIDReportType = HID_DTYPE_Report,
      .HIDReportLength = joystick_hid_report_size
    },
    .HID_JoystickReportINEndpoint = {
      .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress = JOYSTICK_IN_EPADDR,
      .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize = HID_EPSIZE,
      .PollingIntervalMS = 0x01
    },
    .HID_JoystickReportOUTEndpoint = {
      .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress = JOYSTICK_OUT_EPADDR,
      .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize = HID_EPSIZE,
      .PollingIntervalMS = 0x01
    },
    .HID_KeyboardInterface = {
      .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber = INTERFACE_ID_Keyboard,
      .AlternateSetting = 0x00,
      .TotalEndpoints = 1,
      .Class = HID_CSCP_HIDClass,
      .SubClass = HID_CSCP_NonBootSubclass,
      .Protocol = HID_CSCP_NonBootProtocol,
      .InterfaceStrIndex = NO_DESCRIPTOR
    },
    .HID_KeyboardHID = {
      .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
      .HIDSpec = VERSION_BCD(1,1,1),
      .CountryCode = 0x00,
      .TotalReportDescriptors = 1,
      .HIDReportType = HID_DTYPE_Report,
      .HIDReportLength = sizeof(KeyboardHIDReport)
    },
    .HID_KeyboardReportINEndpoint = {
      .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress = KEYBOARD_IN_EPADDR,
      .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize = HID_EPSIZE,
      .PollingIntervalMS = 0x01
    },
    .HID_MouseInterface = {
      .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber = INTERFACE_ID_Mouse,
      .AlternateSetting = 0x00,
      .TotalEndpoints = 1,
      .Class = HID_CSCP_HIDClass,
      .SubClass = HID_CSCP_NonBootSubclass,
      .Protocol = HID_CSCP_NonBootProtocol,
      .InterfaceStrIndex = NO_DESCRIPTOR
    },
    .HID_MouseHID = {
      .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
      .HIDSpec = VERSION_BCD(1,1,1),
      .CountryCode = 0x00,
      .TotalReportDescriptors = 1,
      .HIDReportType = HID_DTYPE_Report,
      .HIDReportLength = sizeof(MouseHIDReport)
    },
    .HID_MouseReportINEndpoint = {
      .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress = MOUSE_IN_EPADDR,
      .Attributes = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize = HID_EPSIZE,
      .PollingIntervalMS = 0x01
    },
    .HID_ConfigInterface = {
      .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber = INTERFACE_ID_Config,
      .AlternateSetting = 0x00,
      .TotalEndpoints = 0,
      .Class = HID_CSCP_HIDClass,
      .SubClass = HID_CSCP_NonBootSubclass,
      .Protocol = HID_CSCP_NonBootProtocol,
      .InterfaceStrIndex = NO_DESCRIPTOR
    },
    .HID_ConfigHID = {
      .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
      .HIDSpec = VERSION_BCD(1,1,1),
      .CountryCode = 0x00,
      .TotalReportDescriptors = 1,
      .HIDReportType = HID_DTYPE_Report,
      .HIDReportLength = sizeof(ConfigHIDReport)
    }
  };
}

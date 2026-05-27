#pragma once

#include "config.h"

//@formatter:off
#define LedStringBase 0x02

#define HID_STRING_INDEX(x) HID_REPORT_ITEM(x, RI_LOCAL_STRING_INDEX, RI_TYPE_LOCAL, 1)

#define HID_PADDING_INPUT(Number) \
HID_REPORT_SIZE(Number), \
HID_REPORT_COUNT(0x01), \
HID_INPUT(HID_CONSTANT)

#define HID_PADDING_OUTPUT(Number) \
HID_REPORT_SIZE(Number), \
HID_REPORT_COUNT(0x01), \
HID_OUTPUT(HID_CONSTANT)

#define HID_BUTTONS(Buttons) \
HID_USAGE_PAGE(0x09), \
HID_USAGE_MIN(0x01), \
HID_USAGE_MAX(Buttons), \
HID_LOGICAL_MIN(0x00), \
HID_LOGICAL_MAX(0x01), \
HID_REPORT_SIZE(0x01), \
HID_REPORT_COUNT(Buttons), \
HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
HID_PADDING_INPUT((Buttons % 8) ? (8 - (Buttons % 8)) : 0)

#define HID_BUTTON_LIGHT(Number) \
HID_USAGE_PAGE(0x0A), \
HID_USAGE(Number), \
HID_COLLECTION(0x02), \
    HID_USAGE_PAGE(0x09), \
    HID_USAGE(Number), \
    HID_STRING_INDEX(LedStringBase+Number), \
    HID_REPORT_SIZE(0x01), \
    HID_REPORT_COUNT(0x01), \
    HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
HID_COLLECTION_END

#define HID_RGB(Number) \
HID_LOGICAL_MIN(0x00), \
HID_LOGICAL_MAX(0xFF), \
HID_USAGE_PAGE(0x0A), \
HID_USAGE(1), \
HID_COLLECTION(0x02), \
    HID_USAGE_PAGE(0x08), \
    HID_USAGE(0x4B), \
    HID_STRING_INDEX(LedStringBase+Number), \
    HID_REPORT_SIZE(0x08), \
    HID_REPORT_COUNT(0x01), \
    HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
HID_COLLECTION_END, \
HID_USAGE(2), \
HID_COLLECTION(0x02), \
    HID_USAGE_PAGE(0x08), \
    HID_USAGE(0x4B), \
    HID_STRING_INDEX(LedStringBase+Number+1), \
    HID_REPORT_SIZE(0x08), \
    HID_REPORT_COUNT(0x01), \
    HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
HID_COLLECTION_END, \
HID_USAGE(3), \
HID_COLLECTION(0x02), \
    HID_USAGE_PAGE(0x08), \
    HID_USAGE(0x4B), \
    HID_STRING_INDEX(LedStringBase+Number+2), \
    HID_REPORT_SIZE(0x08), \
    HID_REPORT_COUNT(0x01), \
    HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
HID_COLLECTION_END
//@formatter:on

enum
{
    ITF_NUM_CDC_0,
    ITF_NUM_CDC_1,
    ITF_NUM_HID,
    ITF_NUM_CONFIG,
    ITF_NUM_LIGHTS,
    ITF_NUM_TOTAL
};

constexpr uint8_t ITF_HID_BASE = ITF_NUM_HID;

#define EPNUM_CDC_CMD   0x81
#define EPNUM_CDC       0x02
#define EPNUM_HID       0x03
#define EPNUM_CONFIG    0x84
#define EPNUM_LIGHTS    0x05

// String Descriptor Index
enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_TOTAL,
};

constexpr uint8_t KEYBOARD_KEYS = 13;

enum
{
    REPORT_ID_JOYSTICK = 1,
    REPORT_ID_KEYBOARD,
    REPORT_ID_MOUSE,
    REPORT_ID_COUNT
};

//@formatter:off
#define HID_REPORT_DESC_KEYBOARD \
HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
HID_USAGE(HID_USAGE_DESKTOP_KEYBOARD), \
HID_COLLECTION(HID_COLLECTION_APPLICATION), \
    HID_REPORT_ID(REPORT_ID_KEYBOARD) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD), \
    HID_USAGE_MIN(0x00), \
    HID_USAGE_MAX_N(0xFF, 2), \
    HID_LOGICAL_MIN(0x00), \
    HID_LOGICAL_MAX_N(0xFF, 2), \
    HID_REPORT_COUNT(KEYBOARD_KEYS), \
    HID_REPORT_SIZE(0x08), \
    HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
HID_COLLECTION_END

#define HID_REPORT_DESC_MOUSE \
HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
HID_USAGE(HID_USAGE_DESKTOP_MOUSE), \
HID_COLLECTION(HID_COLLECTION_APPLICATION), \
    HID_REPORT_ID(REPORT_ID_MOUSE) \
    HID_USAGE(HID_USAGE_DESKTOP_POINTER), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        HID_USAGE(HID_USAGE_DESKTOP_X), \
        HID_USAGE(HID_USAGE_DESKTOP_Y), \
        HID_LOGICAL_MIN(0x81), \
        HID_LOGICAL_MAX(0x7F), \
        HID_REPORT_COUNT(0x02), \
        HID_REPORT_SIZE(0x08), \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE), \
    HID_COLLECTION_END, \
HID_COLLECTION_END
//@formatter:on

enum
{
    REPORT_ID_CONFIG = 0x01,
    REPORT_ID_COMMAND,
    REPORT_ID_FWVER
};

//@formatter:off
constexpr uint8_t desc_config_report[] =
{
    HID_USAGE_PAGE_N(0xFFFB, 2),
    HID_USAGE(0x01),
    HID_LOGICAL_MIN(0x00),
    HID_LOGICAL_MAX(0xFF),
    HID_COLLECTION(0x01),
        HID_REPORT_ID(REPORT_ID_CONFIG)
        HID_USAGE(0x01),
        HID_REPORT_COUNT(sizeof(config_t) - sizeof(uint32_t)),
        HID_REPORT_SIZE(0x08),
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        HID_REPORT_ID(REPORT_ID_COMMAND)
        HID_USAGE(0x02),
        HID_REPORT_COUNT(0x01),
        HID_REPORT_SIZE(0x08),
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        HID_REPORT_ID(REPORT_ID_FWVER)
        HID_USAGE(0x03),
        HID_REPORT_COUNT(0x01),
        HID_REPORT_SIZE(0x20),
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END
};
//@formatter:on

// Interface number, string index, protocol, report descriptor len, EP Out address, size & polling interval
#define CONFIG_DESCRIPTOR \
TUD_HID_DESCRIPTOR(ITF_NUM_CONFIG, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_config_report), EPNUM_CONFIG, \
                   CFG_TUD_HID_EP_BUFSIZE, 4)

constexpr tusb_desc_device_t generate_device_descriptor(uint16_t vid, uint16_t pid)
{
    return
    {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = vid,
        .idProduct = pid,
        .bcdDevice = 0x0200,

        .iManufacturer = 0x01,
        .iProduct = 0x02,

        .bNumConfigurations = 0x01
    };
}

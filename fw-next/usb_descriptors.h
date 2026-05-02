#pragma once

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

// HID Output only descriptor
// Interface number, string index, protocol, report descriptor len, EP Out address, size & polling interval
#define TUD_HID_OUT_DESCRIPTOR(_itfnum, _stridx, _boot_protocol, _report_desc_len, _epout, _epsize, _ep_interval) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_HID, (uint8_t)((_boot_protocol != HID_ITF_PROTOCOL_NONE) ? (uint8_t)HID_SUBCLASS_BOOT : 0u), _boot_protocol, _stridx,\
  /* HID descriptor */\
  9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0111), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(_report_desc_len),\
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval

enum
{
    ITF_NUM_CDC_0,
    ITF_NUM_CDC_1,
    ITF_NUM_HID,
    ITF_NUM_LIGHTS,
    ITF_NUM_TOTAL
};

#define EPNUM_CDC_CMD   0x81
#define EPNUM_CDC       0x02
#define EPNUM_HID       0x03
#define EPNUM_LIGHTS    0x04

// String Descriptor Index
enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_TOTAL,
};

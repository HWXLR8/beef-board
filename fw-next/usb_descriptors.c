#include "bsp/board_api.h"
#include "tusb.h"

#define USB_PID   0x8048
#define USB_VID   0x1CCF
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
static tusb_desc_device_t const desc_device =
{
  .bLength = sizeof(tusb_desc_device_t),
  .bDescriptorType = TUSB_DESC_DEVICE,
  .bcdUSB = USB_BCD,
  .bDeviceClass = 0x00,
  .bDeviceSubClass = 0x00,
  .bDeviceProtocol = 0x00,
  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor = USB_VID,
  .idProduct = USB_PID,
  .bcdDevice = 0x0100,

  .iManufacturer = 0x01,
  .iProduct = 0x02,
  .iSerialNumber = 0,

  .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void)
{
  return (uint8_t const*)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

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

const uint8_t desc_hid_report[] =
{
  HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
  HID_USAGE(HID_USAGE_DESKTOP_JOYSTICK),
  HID_COLLECTION(HID_COLLECTION_APPLICATION),
    // Analog
    HID_USAGE(HID_USAGE_DESKTOP_POINTER),
    HID_COLLECTION(HID_COLLECTION_LOGICAL),
      HID_USAGE(HID_USAGE_DESKTOP_X),
      HID_USAGE(HID_USAGE_DESKTOP_Y),
      HID_LOGICAL_MIN_N(0, 2),
      HID_LOGICAL_MAX_N(255, 2),
      HID_PHYSICAL_MIN(-1),
      HID_PHYSICAL_MAX(1),
      HID_REPORT_COUNT(0x02),
      HID_REPORT_SIZE(0x08),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

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
  HID_COLLECTION_END
};
//@formatter:on

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf)
{
  (void)itf;
  return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)

#define EPNUM_HID   0x01

uint8_t const desc_configuration[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 500),

  // Interface number, string index, protocol, report descriptor len, EP Out & In address, size & polling interval
  TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, 0x80 | EPNUM_HID,
                           CFG_TUD_HID_EP_BUFSIZE, 1)
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index; // for multiple configurations

  // This example use the same configuration for both high and full speed mode
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum
{
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_TOTAL,
};

// array of pointer to string descriptors
static char const* string_desc_arr[] =
{
  (const char[]){ 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "SEGV",                       // 1: Manufacturer
  "BEEF BOARD V2",              // 2: Product
  "Button 1",
  "Button 2",
  "Button 3",
  "Button 4",
  "Button 5",
  "Button 6",
  "Button 7",
  "Button 8",
  "Button 9",
  "Button 10",
  "Button 11",
  "Turntable R",
  "Turntable G",
  "Turntable B",
  "Centre Bar R",
  "Centre Bar G",
  "Centre Bar B"
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;
  size_t chr_count;

  switch (index)
  {
  case STRID_LANGID:
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
    break;

  default:
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
    if (chr_count > max_count) chr_count = max_count;

    // Convert ASCII string into UTF-16
    for (size_t i = 0; i < chr_count; i++)
    {
      _desc_str[1 + i] = str[i];
    }
    break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}

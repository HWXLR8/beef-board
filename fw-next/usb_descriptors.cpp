#include "usb_descriptors.h"

#include "beef.h"
#include "tusb.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb()
{
    return (uint8_t const*)usb->get_descriptor_device();
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf)
{
    (void)itf;
    return usb->get_hid_descriptor_report();
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations

    // This example use the same configuration for both high and full speed mode
    return usb->get_descriptor_configuration();
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
static char const* string_desc_arr[] =
{
    (const char[]){ 0x09, 0x04 }, // 0: is supported language is English (0x0409)
    "SEGV",                       // 1: Manufacturer
    "BEEF BOARD V2"               // 2: Product
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    const char* str = nullptr;

    switch (index)
    {
    case STRID_LANGID:
    case STRID_MANUFACTURER:
    case STRID_PRODUCT:
        str = string_desc_arr[index];
        break;
    default:
        str = usb->get_descriptor_string(index - LedStringBase - 1);
        break;
    }

    if (str == nullptr) return nullptr;

    // Cap at max char
    size_t chr_count = strlen(str);
    constexpr size_t max_count = std::size(_desc_str) - 1; // -1 for string type
    if (chr_count > max_count) chr_count = max_count;

    // Convert ASCII string into UTF-16
    for (size_t i = 0; i < chr_count; i++)
    {
        _desc_str[1 + i] = str[i];
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return _desc_str;
}

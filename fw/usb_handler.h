#pragma once

#include <LUFA/Drivers/USB/Class/Device/HIDClassDevice.h>
#include "Descriptors.h"

#include "config.h"
#include "hid.h"

class AbstractUsbHandler {
public:
  virtual bool create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                                 uint8_t* const report_id,
                                 const uint8_t report_type,
                                 void* report_data,
                                 uint16_t* const report_size) = 0;
  virtual void update(config &config) = 0;
};

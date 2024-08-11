#pragma once

#include <LUFA/Drivers/USB/Class/Device/HIDClassDevice.h>
#include "Descriptors.h"

#include "config.h"

class AbstractUsbHandler {
public:
  virtual bool create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                                 uint8_t* const report_id,
                                 const uint8_t report_type,
                                 void* report_data,
                                 uint16_t* const report_size) = 0;
  virtual void usb_task(const config &config) = 0;
  virtual void update(const config &config,
                      timer &hid_lights_expiry_timer,
                      timer &combo_lights_timer) = 0;
};

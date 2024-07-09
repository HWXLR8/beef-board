#pragma once

#include "../config.h"
#include "sdvx_rgb.h"

namespace SDVX {
  class UsbHandler : public AbstractUsbHandler {
  public:
    UsbHandler() = default;

    bool create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                           uint8_t* const report_id,
                           const uint8_t report_type,
                           void* report_data,
                           uint16_t* const report_size) override;
    void update(config &config) override;

  private:
    hid_lights led_data{};
  };

  void usb_init(config &config);
  void update_hid_interface();
}

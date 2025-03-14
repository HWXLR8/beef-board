#pragma once

#include "../config.h"
#include "../usb_handler.h"
#include "sdvx_rgb_manager.h"

namespace SDVX {
  class UsbHandler : public AbstractUsbHandler {
  public:
    UsbHandler() = default;

    bool create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                           uint8_t* const report_id,
                           void* report_data,
                           uint16_t* const report_size) override;
    void usb_task(const config &config) override;
    void update(const config &config) override;

  private:
    hid_lights led_data{};
  };

  void usb_init(const config &config);
}

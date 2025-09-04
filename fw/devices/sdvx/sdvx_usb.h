#pragma once

#include "../config.h"
#include "../usb_handler.h"
#include "sdvx_rgb_manager.h"

namespace SDVX {
  class UsbHandler : public AbstractUsbHandler {
  public:
    UsbHandler() = default;

    bool create_hid_report(USB_ClassInfo_HID_Device_t* hid_interface_info,
                           uint8_t* report_id,
                           void* report_data,
                           uint16_t* report_size) override;
    void usb_task(const config &config) override;
    void update(const config &config) override;
    void config_update(const config &new_config) override;

  private:
    hid_lights led_data{};
  };

  void usb_init(const config &config);
}

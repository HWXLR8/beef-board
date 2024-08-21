#pragma once

#include "../beef.h"
#include "../config.h"
#include "../usb_handler.h"
#include "iidx_rgb_manager.h"

namespace IIDX {
  class UsbHandler : public AbstractUsbHandler {
  public:
    UsbHandler() = default;

    bool create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                           uint8_t* const report_id,
                           const uint8_t report_type,
                           void* report_data,
                           uint16_t* const report_size) override;
    void usb_task(const config &config) override;
    void update() override;

  private:
    hid_lights led_data{};
  };

  void usb_init(const config &config);

  enum {
    BUTTON_TT_NEG = 1 << 11,
    BUTTON_TT_POS = 1 << 12,
    EFFECTORS_ALL = BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11
  };
}

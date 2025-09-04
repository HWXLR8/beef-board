#pragma once

#include "../beef.h"
#include "../config.h"
#include "../usb_handler.h"
#include "iidx_rgb_manager.h"

namespace IIDX {
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

  enum {
    BUTTON_TT_NEG = 1 << 11,
    BUTTON_TT_POS = 1 << 12,
    MAIN_BUTTONS_ALL = BUTTON_1 | BUTTON_2 | BUTTON_3 | BUTTON_4 | BUTTON_5 | BUTTON_6 | BUTTON_7,
    EFFECTORS_ALL = BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11
  };
}

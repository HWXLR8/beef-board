#include "../analog_turntable.h"
#include "../beef.h"
#include "../hid.h"
#include "iidx_combo.h"
#include "iidx_usb.h"
#include "iidx_usb_desc.h"
#include "iidx_rgb.h"
#include "iidx_rgb_manager.h"

namespace IIDX {
  struct USB_JoystickReport_Data_t {
    uint8_t  X;
    uint16_t Button; // bit-field representing which buttons have been pressed
  };

  HidReport<USB_JoystickReport_Data_t> hid_report;

  hid_lights led_state_from_hid_report{};

  bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const,
                                           uint8_t* const,
                                           const uint8_t,
                                           void* ReportData,
                                           uint16_t* const ReportSize) {
    auto JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

    // Infinitas only reads bits 1-7, 9-12,
    // so bit-shift bits 8 and up once
    const uint8_t upper = button_state >> 7;
    const uint8_t lower = button_state & 0x7F;
    JoystickReport->X = tt_x.tt_position / current_config.tt_ratio;
    JoystickReport->Button = (upper << 8) | lower;

    *ReportSize = sizeof(USB_JoystickReport_Data_t);
    return true;
  }

  void process_buttons(int8_t tt1_report) {
    static DebounceState effectors_debounce(4);

    switch (tt1_report) {
      case -1:
        button_state |= BUTTON_TT_NEG;
        break;
      case 1:
        button_state |= BUTTON_TT_POS;
        break;
      default:
        break;
    }

    debounce(&effectors_debounce, EFFECTORS_ALL);
  }

  void update(const config &config) {
    HID_Task(&led_state_from_hid_report);

    process_tt(tt_x, config.tt_ratio);
    const auto tt1_report = analog_turntable_poll(&tt1, tt_x.tt_position);
    process_buttons(tt1_report);

    update_lighting(led_state_from_hid_report.buttons);
    RgbManager::update(config,
                       tt1_report,
                       led_state_from_hid_report);
  }

  void usb_init(config &config) {
    hid_interface = &hid_report.Joystick_HID_Interface;
    update_callback = update;
    usb_desc_callback = CALLBACK_USB_GetDescriptor;
    create_hid_report_callback = CALLBACK_HID_Device_CreateHIDReport;
    get_button_combo_callback = get_button_combo;

    update_tt_transitions(config.reverse_tt);
  }
}

#include "../beef.h"
#include "../hid.h"
#include "sdvx_combo.h"
#include "sdvx_rgb.h"
#include "sdvx_usb.h"
#include "sdvx_usb_desc.h"

namespace Sdvx {
  struct USB_JoystickReport_Data_t {
    uint8_t X;
    uint8_t Y;
    uint16_t Button; // bit-field representing which buttons have been pressed
  };

  HidReport<USB_JoystickReport_Data_t> hid_report;

  hid_lights led_state_from_hid_report{};

  void (*get_axis) (USB_JoystickReport_Data_t*);
  void (*update_analog) (const config &);

  void get_axis_encoder(USB_JoystickReport_Data_t* report_data) {
    report_data->X = encoder_x.position;
    report_data->Y = encoder_y.position;
  }

  void get_axis_tt(USB_JoystickReport_Data_t* report_data) {
    report_data->X = tt_x.tt_position;
    report_data->Y = tt_y.tt_position;
  }

  void update_analog_encoder(const config &config) {
    process_encoder(encoder_x);
    process_encoder(encoder_y);
  }

  void update_analog_tt(const config &config) {
    process_tt(tt_x, config.tt_ratio);
    process_tt(tt_y, config.tt_ratio);
  }

  bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const,
                                           uint8_t* const,
                                           const uint8_t,
                                           void* ReportData,
                                           uint16_t* const ReportSize) {
    auto JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

    get_axis(JoystickReport);
    JoystickReport->Button = button_state;

    *ReportSize = sizeof(USB_JoystickReport_Data_t);
    return true;
  }

  void update(const config &config) {
    HID_Task(&led_state_from_hid_report);
    update_analog(config);

    //const auto start_button = led_state_from_hid_report.buttons & BUTTON_7;
    //led_state_from_hid_report.buttons &= 0x3F; // Clear start button light state
    //led_state_from_hid_report.buttons |= start_button << 2;
    update_lighting(led_state_from_hid_report.buttons);
  }

  void usb_init(config &config) {
    hid_interface = &hid_report.Joystick_HID_Interface;
    update_callback = update;
    usb_desc_callback = CALLBACK_USB_GetDescriptor;
    create_hid_report_callback = CALLBACK_HID_Device_CreateHIDReport;
    get_button_combo_callback = get_button_combo;

    get_axis = get_axis_encoder;
    update_analog = update_analog_encoder;

    update_tt_transitions(false);
  }
}

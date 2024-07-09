#include "../analog_button.h"
#include "../axis.h"
#include "../beef.h"
#include "sdvx_combo.h"
#include "sdvx_rgb.h"
#include "sdvx_usb.h"
#include "sdvx_usb_desc.h"

namespace SDVX {
  struct USB_JoystickReport_Data_t {
    uint8_t  X;
    uint8_t  Y;
    uint16_t Button; // bit-field representing which buttons have been pressed
  };
  struct USB_KeyboardReport_Data_t {
    uint8_t X;
    uint8_t Y;
    uint8_t KeyCode[KEYBOARD_KEYS];
  };

  uint8_t key_codes[KEYBOARD_KEYS] = {
    HID_KEYBOARD_SC_D, // BT-A
    HID_KEYBOARD_SC_F, // BT-B
    HID_KEYBOARD_SC_J, // BT-C
    HID_KEYBOARD_SC_K, // BT-D
    HID_KEYBOARD_SC_C, // FX-L
    HID_KEYBOARD_SC_M, // FX-R
    HID_KEYBOARD_SC_ENTER // Start
  };

  static_assert(sizeof(USB_KeyboardReport_Data_t) >= sizeof(USB_JoystickReport_Data_t), "");
  HidReport<USB_KeyboardReport_Data_t> hid_report;
  UsbHandler usb_handler;

  Axis* axis_x;
  Axis* axis_y;

  bool UsbHandler::create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                                     uint8_t* const report_id,
                                     const uint8_t report_type,
                                     void* report_data,
                                     uint16_t* const report_size) {
    switch (current_config.sdvx_input_mode) {
      case InputMode::Joystick: {
        auto joystick_report = (USB_JoystickReport_Data_t*)report_data;
        *report_size = sizeof(*joystick_report);
        *report_id = HID_REPORTID_JoystickReport;

        joystick_report->X = axis_x->get();
        joystick_report->Y = axis_y->get();
        joystick_report->Button = button_state;

        return true;
      }
      case InputMode::Keyboard: {
        auto keyboard_report = (USB_KeyboardReport_Data_t*)report_data;
        *report_size = sizeof(*keyboard_report);
        *report_id = HID_REPORTID_KeyboardReport;

        process_keyboard(keyboard_report->KeyCode, key_codes, KEYBOARD_KEYS);
        keyboard_report->X = button_x.state;
        keyboard_report->Y = button_y.state;

        return false;
      }
    }
  }

  void UsbHandler::update(config &config) {
    HID_Task(led_data);

    axis_x->poll();
    axis_y->poll();
    analog_button_poll(&button_x, axis_x->get());
    analog_button_poll(&button_y, axis_y->get());

    update_lighting(led_data.buttons);
  }

  void usb_init(config &config) {
    usb_desc_init();

    ::usb_handler = &usb_handler;
    get_button_combo_callback = get_button_combo;

    axis_x = &analog_x;
    axis_x = &analog_y;

    update_tt_transitions(false);
  }

  void update_hid_interface() {
    hid_interface = &hid_report.HID_Interface;
  }
}

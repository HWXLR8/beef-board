#include "../analog_button.h"
#include "../axis.h"
#include "../beef.h"
#include "sdvx_combo.h"
#include "sdvx_usb.h"
#include "sdvx_usb_desc.h"

namespace SDVX {
  struct USB_JoystickReport_Data_t {
    uint8_t  X;
    uint8_t  Y;
    uint16_t Button; // bit-field representing which buttons have been pressed
  } ATTR_PACKED;

  enum {
    KEYBOARD_KEYS = 9
  };
  const uint8_t key_codes[KEYBOARD_KEYS] = {
    HID_KEYBOARD_SC_D, // BT-A
    HID_KEYBOARD_SC_F, // BT-B
    HID_KEYBOARD_SC_J, // BT-C
    HID_KEYBOARD_SC_K, // BT-D
    HID_KEYBOARD_SC_C, // FX-L
    HID_KEYBOARD_SC_M, // FX-R
    0,
    0,
    HID_KEYBOARD_SC_ENTER // Start
  };

  HidReport<USB_JoystickReport_Data_t, INTERFACE_ID_Joystick, JOYSTICK_IN_EPADDR> joystick_hid_report;
  HidReport<Beef::USB_KeyboardReport_Data_t, INTERFACE_ID_Keyboard, KEYBOARD_IN_EPADDR> keyboard_hid_report;
  HidReport<Beef::USB_MouseReport_Data_t, INTERFACE_ID_Mouse, MOUSE_IN_EPADDR> mouse_hid_report;
  UsbHandler usb_handler;

  Axis* axis_x;
  Axis* axis_y;

  bool UsbHandler::create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                                     uint8_t* const report_id,
                                     const uint8_t report_type,
                                     void* report_data,
                                     uint16_t* const report_size) {
    switch (hid_interface_info->Config.ReportINEndpoint.Address) {
      case JOYSTICK_IN_EPADDR: {
        auto joystick_report = (USB_JoystickReport_Data_t*)report_data;
        *report_size = sizeof(*joystick_report);

        joystick_report->X = axis_x->get();
        joystick_report->Y = axis_y->get();
        joystick_report->Button = button_state;

        return true;
      }
      case KEYBOARD_IN_EPADDR: {
        auto keyboard_report = (Beef::USB_KeyboardReport_Data_t*)report_data;
        *report_size = sizeof(*keyboard_report);

        process_keyboard(keyboard_report, key_codes, KEYBOARD_KEYS);

        return false;
      }
      case MOUSE_IN_EPADDR: {
        auto mouse_report = (Beef::USB_MouseReport_Data_t*)report_data;
        *report_size = sizeof(*mouse_report);

        mouse_report->X = button_x.direction;
        mouse_report->Y = button_y.direction;

        return false;
      }
      default:
        return false;
    }
  }

  void UsbHandler::usb_task(const config &config) {
    switch (config.sdvx_input_mode) {
      case InputMode::Joystick:
        HID_Device_USBTask(&joystick_hid_report.HID_Interface);
        break;
      case InputMode::Keyboard:
        HID_Device_USBTask(&keyboard_hid_report.HID_Interface);
        HID_Device_USBTask(&mouse_hid_report.HID_Interface);
        break;
    }
  }

  void UsbHandler::update() {
    HID_Task(led_data);

    axis_x->poll();
    axis_y->poll();
    analog_button_poll(&button_x, axis_x->get());
    analog_button_poll(&button_y, axis_y->get());

    update_button_lighting(led_data.buttons);
  }

  void usb_init(const config &config) {
    usb_desc_init();

    ::usb_handler = &usb_handler;
    get_button_combo_callback = get_button_combo;

    axis_x = &analog_x;
    axis_y = &analog_y;
    analog_button_init(&button_x, 1, 0, false);
    analog_button_init(&button_y, 1, 0, false);

    update_tt_transitions(false);
  }
}

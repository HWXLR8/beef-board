#include <analog_button.h>
#include "../beef.h"
#include "../hid.h"
#include "sdvx_combo.h"
#include "sdvx_rgb.h"
#include "sdvx_usb.h"
#include "sdvx_usb_desc.h"

namespace SDVX {
  struct USB_JoystickReport_Data_t {
    uint8_t X;
    uint8_t Y;
    uint16_t Button; // bit-field representing which buttons have been pressed
  };
  struct USB_KeyboardReport_Data_t {
    uint8_t KeyCode[KEYBOARD_KEYS];
  };

  uint8_t key_codes[KEYBOARD_KEYS] = {
    HID_KEYBOARD_SC_D, // BT-A
    HID_KEYBOARD_SC_F, // BT-B
    HID_KEYBOARD_SC_J, // BT-C
    HID_KEYBOARD_SC_K, // BT-D
    HID_KEYBOARD_SC_C, // FX-L
    HID_KEYBOARD_SC_M, // FX-R
    HID_KEYBOARD_SC_S, // VOL-L -
    HID_KEYBOARD_SC_A, // VOL-L +
    HID_KEYBOARD_SC_ENTER, // Start
    HID_KEYBOARD_SC_SEMICOLON_AND_COLON, // // VOL-R -
    HID_KEYBOARD_SC_L// VOL-R +
  };

  HidReport<USB_JoystickReport_Data_t, INTERFACE_ID_Joystick, JOYSTICK_IN_EPADDR> joystick_report;
  HidReport<USB_KeyboardReport_Data_t, INTERFACE_ID_Keyboard, KEYBOARD_IN_EPADDR> keyboard_report;

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

  void process_buttons(const int8_t analog_x,
                       const int8_t analog_y) {
    switch (analog_x) {
      case -1:
        button_state |= BUTTON_ANALOG_X_NEG;
        break;
      case 1:
        button_state |= BUTTON_ANALOG_X_POS;
        break;
      default:
        break;
    }
    switch (analog_y) {
      case -1:
        button_state |= BUTTON_ANALOG_Y_NEG;
        break;
      case 1:
        button_state |= BUTTON_ANALOG_Y_POS;
        break;
      default:
        break;
    }
  }

  void update_analog_encoder(const config &config) {
    process_encoder(encoder_x);
    process_encoder(encoder_y);

    const auto analog_x = analog_button_poll(&button_x, encoder_x.position);
    const auto analog_y = analog_button_poll(&button_y, encoder_y.position);

    process_buttons(analog_x, analog_y);
  }

  void update_analog_tt(const config &config) {
    process_tt(tt_x, 1);
    process_tt(tt_y, 1);

    const auto ttx_report = analog_button_poll(&button_x, tt_x.tt_position);
    const auto tty_report = analog_button_poll(&button_y, tt_y.tt_position);
    process_buttons(ttx_report, tty_report);
  }

  bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                           uint8_t* const,
                                           const uint8_t,
                                           void* ReportData,
                                           uint16_t* const ReportSize) {
    switch (HIDInterfaceInfo->Config.InterfaceNumber) {
      case INTERFACE_ID_Joystick: {
        auto JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

        get_axis(JoystickReport);
        JoystickReport->Button = button_state;

        *ReportSize = sizeof(*JoystickReport);
        return true;
      }
      case INTERFACE_ID_Keyboard: {
        auto KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

        process_keyboard(KeyboardReport->KeyCode, key_codes, KEYBOARD_KEYS);

        *ReportSize = sizeof(*KeyboardReport);
        return false;
      }
      default:
        return false;
    }
  }

  void update(const config &config) {
    HID_Task(&led_state_from_hid_report);

    update_analog(config);

    update_lighting(led_state_from_hid_report.buttons);
  }

  void usb_init(config &config) {
    usb_desc_init();

    update_callback = update;
    create_hid_report_callback = CALLBACK_HID_Device_CreateHIDReport;
    get_button_combo_callback = get_button_combo;

    get_axis = get_axis_encoder;
    update_analog = update_analog_encoder;

    update_tt_transitions(false);
  }

  void update_hid_interface(config &config) {
    switch (config.sdvx_input_mode) {
      case InputMode::Joystick:
        hid_interface = &joystick_report.HID_Interface;
        break;
      case InputMode::Keyboard:
        hid_interface = &keyboard_report.HID_Interface;
        break;
    }
  }
}

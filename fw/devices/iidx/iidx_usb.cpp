#include "../analog_button.h"
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
  struct USB_KeyboardReport_Data_t {
    uint8_t KeyCode[KEYBOARD_KEYS];
  };

  uint8_t key_codes[KEYBOARD_KEYS] = {
    HID_KEYBOARD_SC_S, // 1
    HID_KEYBOARD_SC_D, // 2
    HID_KEYBOARD_SC_F, // 3
    HID_KEYBOARD_SC_SPACE, // 4
    HID_KEYBOARD_SC_J, // 5
    HID_KEYBOARD_SC_K, // 6
    HID_KEYBOARD_SC_L, // 7
    HID_KEYBOARD_SC_1_AND_EXCLAMATION, // E1/Start
    HID_KEYBOARD_SC_2_AND_AT, // E2
    HID_KEYBOARD_SC_3_AND_HASHMARK, // E3
    HID_KEYBOARD_SC_4_AND_DOLLAR, // E4/Select
    HID_KEYBOARD_SC_DOWN_ARROW, // TT -
    HID_KEYBOARD_SC_UP_ARROW // TT +
  };

  HidReport<USB_JoystickReport_Data_t, INTERFACE_ID_Joystick, JOYSTICK_IN_EPADDR> joystick_report;
  HidReport<USB_KeyboardReport_Data_t, INTERFACE_ID_Keyboard, KEYBOARD_IN_EPADDR> keyboard_report;

  hid_lights led_state_from_hid_report{};

  bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                           uint8_t* const,
                                           const uint8_t,
                                           void* ReportData,
                                           uint16_t* const ReportSize) {
    switch (HIDInterfaceInfo->Config.InterfaceNumber) {
      case INTERFACE_ID_Joystick: {
        auto JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

        // Infinitas only reads buttons 1-7, 9-12,
        // so shift bits 8 and up once
        const uint8_t upper = button_state >> 7;
        const uint8_t lower = button_state & 0x7F;
        JoystickReport->X = tt_x.tt_position / current_config.tt_ratio;
        JoystickReport->Button = (upper << 8) | lower;

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

  void process_buttons(const int8_t tt1_report) {
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
    const auto tt1_report = analog_button_poll(&button_x, tt_x.tt_position);
    process_buttons(tt1_report);

    update_lighting(led_state_from_hid_report.buttons);
    RgbManager::update(config,
                       tt1_report,
                       led_state_from_hid_report);
  }

  void usb_init(config &config) {
    usb_desc_init();

    update_callback = update;
    create_hid_report_callback = CALLBACK_HID_Device_CreateHIDReport;
    get_button_combo_callback = get_button_combo;

    update_tt_transitions(config.reverse_tt);
  }

  void update_hid_interface(config &config) {
    switch (config.iidx_input_mode) {
      case InputMode::Joystick:
        hid_interface = &joystick_report.HID_Interface;
        break;
      case InputMode::Keyboard:
        hid_interface = &keyboard_report.HID_Interface;
        break;
    }
  }
}

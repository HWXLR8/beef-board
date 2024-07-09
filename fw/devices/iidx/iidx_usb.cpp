#include "../analog_button.h"
#include "../axis.h"
#include "../beef.h"
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

  static_assert(sizeof(USB_KeyboardReport_Data_t) >= sizeof(USB_JoystickReport_Data_t), "");
  HidReport<USB_KeyboardReport_Data_t> hid_report;
  UsbHandler usb_handler;

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

  bool UsbHandler::create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                                     uint8_t* const report_id,
                                     const uint8_t report_type,
                                     void* report_data,
                                     uint16_t* const report_size) {
    switch (current_config.iidx_input_mode) {
      case InputMode::Joystick: {
        auto joystick_report = (USB_JoystickReport_Data_t*)report_data;
        *report_size = sizeof(*joystick_report);
        *report_id = HID_REPORTID_JoystickReport;

        // Infinitas only reads buttons 1-7, 9-12,
        // so shift bits 8 and up once
        const uint8_t upper = button_state >> 7;
        const uint8_t lower = button_state & 0x7F;
        joystick_report->X = tt_x.get();
        joystick_report->Button = (upper << 8) | lower;

        return true;
      }
      case InputMode::Keyboard: {
        auto keyboard_report = (USB_KeyboardReport_Data_t*)report_data;
        *report_size = sizeof(*keyboard_report);
        *report_id = HID_REPORTID_KeyboardReport;

        process_keyboard(keyboard_report->KeyCode, key_codes, KEYBOARD_KEYS);

        return false;
      }
    }
  }

  void UsbHandler::update(config &config) {
    HID_Task(led_data);

    tt_x.poll();
    const auto tt1_report = analog_button_poll(&button_x, tt_x.get());
    process_buttons(tt1_report);

    update_lighting(led_data.buttons);
    RgbManager::update(config,
                       tt1_report,
                       led_data);
  }

  void usb_init(config &config) {
    usb_desc_init();

    ::usb_handler = &usb_handler;
    get_button_combo_callback = get_button_combo;

    update_tt_transitions(config.reverse_tt);
  }

  void update_hid_interface() {
    hid_interface = &hid_report.HID_Interface;
  }
}

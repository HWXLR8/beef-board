#include "../analog_button.h"
#include "../axis.h"
#include "../beef.h"
#include "iidx_combo.h"
#include "iidx_usb.h"
#include "iidx_usb_desc.h"
#include "iidx_rgb_manager.h"

namespace IIDX {
  struct USB_JoystickReport_Data_t {
    uint8_t  X;
    uint8_t  Y; // Needed for LR2 compatibility
    uint16_t Button; // bit-field representing which buttons have been pressed
  } ATTR_PACKED;

  enum {
    KEYBOARD_KEYS = 13
  };
  const uint8_t key_codes[KEYBOARD_KEYS] = {
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

  HidReport<USB_JoystickReport_Data_t, INTERFACE_ID_Joystick, JOYSTICK_IN_EPADDR> joystick_hid_report;
  HidReport<Beef::USB_KeyboardReport_Data_t, INTERFACE_ID_Keyboard, KEYBOARD_IN_EPADDR> keyboard_hid_report;
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
    switch (hid_interface_info->Config.ReportINEndpoint.Address) {
      case JOYSTICK_IN_EPADDR: {
        auto joystick_report = (USB_JoystickReport_Data_t*)report_data;
        *report_size = sizeof(*joystick_report);

        // Infinitas only reads buttons 1-7, 9-12,
        // so shift bits 8 and up once
        const uint8_t upper = button_state >> 7;
        const uint8_t lower = button_state & 0x7F;
        joystick_report->X = tt_x.get();
        joystick_report->Y = 127;
        joystick_report->Button = (upper << 8) | lower;

        return true;
      }
      case KEYBOARD_IN_EPADDR: {
        auto keyboard_report = (Beef::USB_KeyboardReport_Data_t*)report_data;
        *report_size = sizeof(*keyboard_report);

        process_keyboard(keyboard_report, key_codes, KEYBOARD_KEYS);

        return false;
      }
      default:
        return false;
    }
  }

  void UsbHandler::usb_task(const config &config) {
    switch (config.iidx_input_mode) {
      case InputMode::Joystick:
        HID_Device_USBTask(&joystick_hid_report.HID_Interface);
        break;
      case InputMode::Keyboard:
        HID_Device_USBTask(&keyboard_hid_report.HID_Interface);
        break;
    }
  }

  void UsbHandler::update(const config &config) {
    HID_Task(led_data);

    tt_x.poll();
    const auto tt1_report = analog_button_poll(&button_x, tt_x.get());
    process_buttons(tt1_report);

    update_button_lighting(led_data.buttons);
    RgbManager::update(config,
                       button_x.state,
                       led_data);
  }

  void usb_init(const config &config) {
    usb_desc_init();

    ::usb_handler = &usb_handler;
    get_button_combo_callback = get_button_combo;

    analog_button_init(&button_x, config.tt_deadzone, 200, true);

    update_tt_transitions(config.reverse_tt);
  }
}

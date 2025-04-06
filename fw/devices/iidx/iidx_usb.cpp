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

  HidReport<USB_JoystickReport_Data_t, INTERFACE_ID_Joystick, JOYSTICK_IN_EPADDR> joystick_hid_report;
  HidReport<Beef::USB_KeyboardReport_Data_t, INTERFACE_ID_Keyboard, KEYBOARD_IN_EPADDR> keyboard_hid_report;
  UsbHandler usb_handler;

  DebounceState effectors_debounce(4);
  void process_buttons(const int8_t tt1_report) {
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

    debounce(effectors_debounce, EFFECTORS_ALL);
  }

  bool UsbHandler::create_hid_report(USB_ClassInfo_HID_Device_t* const hid_interface_info,
                                     uint8_t* const report_id,
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

        process_keyboard(keyboard_report,
                         current_config.iidx_keys.key_codes,
                         sizeof(current_config.iidx_keys.key_codes));

        return false;
      }
      default:
        Endpoint_StallTransaction();
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
    const auto tt1_report = button_x.poll(config.tt_deadzone,
                                          config.tt_sustain_ms,
                                          tt_x.get());
    process_buttons(tt1_report);

    update_button_lighting(led_data.buttons);
    RgbManager::update(tt1_report,led_data);
  }

  void usb_init(const config &config) {
    usb_desc_init();

    ::usb_handler = &usb_handler;
    get_button_combo_callback = get_button_combo;

    button_x.init(config.tt_deadzone, true, tt_x.get());

    update_tt_transitions(config.reverse_tt);
  }
}

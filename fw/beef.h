#pragma once

#include <LUFA/Drivers/USB/USB.h>

#include "config.h"
#include "debounce.h"
#include "pin.h"
#include "usb_handler.h"

extern uint16_t button_state;

extern USB_ClassInfo_HID_Device_t* hid_interface;
extern AbstractUsbHandler* usb_handler;

void hwinit();
void usb_init(config &config);
void set_hid_interface(UsbMode usb_mode);
void hardware_timer1_init();
void set_led(volatile uint8_t* PORT,
             uint8_t button_number,
             uint8_t led_pin,
             uint16_t OutputData);
void set_hid_standby_lighting();
void process_buttons();
void process_button(const volatile uint8_t* PIN,
                    uint8_t button_number,
                    uint8_t input_pin);
void update_tt_transitions(uint8_t reverse_tt);
void process_keyboard(uint8_t* hid_key_codes, const uint8_t* key_codes, uint8_t n);
void update_lighting(uint16_t hid_buttons);
void update_button_lighting(uint16_t led_state);

void debounce(DebounceState* debounce, uint16_t mask);
bool is_only_pressed(uint16_t button_bits, uint16_t ignore = 0);
bool is_pressed(uint16_t button_bits, uint16_t ignore = 0);
void check_for_dfu();
void jump_to_bootloader();

// button macros to map to bit positions within button_state
enum {
  BUTTON_1  = 1 << 0,
  BUTTON_2  = 1 << 1,
  BUTTON_3  = 1 << 2,
  BUTTON_4  = 1 << 3,
  BUTTON_5  = 1 << 4,
  BUTTON_6  = 1 << 5,
  BUTTON_7  = 1 << 6,
  BUTTON_8  = 1 << 7,
  BUTTON_9  = 1 << 8,
  BUTTON_10 = 1 << 9,
  BUTTON_11 = 1 << 10
};

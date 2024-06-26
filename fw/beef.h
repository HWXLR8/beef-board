#pragma once

#include <LUFA/Drivers/USB/USB.h>

#include "config.h"
#include "debounce.h"
#include "pin.h"

extern uint16_t button_state;
extern bool reactive_led;
extern bool rgb_standby;
extern timer hid_lights_expiry_timer;
extern config current_config;

extern tt_pins tt_x;
extern tt_pins tt_y;
extern encoder_pin encoder_x;
extern encoder_pin encoder_y;

extern USB_ClassInfo_HID_Device_t* hid_interface;

extern void (*update_callback) (const config &);
extern uint16_t (*usb_desc_callback) (const uint16_t,
                                      const uint16_t,
                                      const void** const);
extern bool (*create_hid_report_callback) (USB_ClassInfo_HID_Device_t* const,
                                           uint8_t* const,
                                           const uint8_t,
                                           void*,
                                           uint16_t* const);

void hwinit();
void usb_init(config &config);
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
void process_tt(tt_pins &tt_pin, uint8_t tt_ratio);
void process_encoder(encoder_pin &encoder_pin);
void update_lighting(uint16_t hid_buttons);
void update_button_lighting(uint16_t led_state);

void debounce(DebounceState* debounce, uint16_t mask);
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

#pragma once

#include "bpm.h"
#include "pin.h"
#include "rgb.h"
#include "timer.h"

// HID report structure, for creating and sending HID reports to the
// host PC. This mirrors the layout described to the host in the HID
// report descriptor in Descriptors.c.
typedef struct {
  uint8_t  X;
  uint16_t Button; // bit-field representing which buttons have been pressed
} USB_JoystickReport_Data_t;

extern uint16_t button_state;
extern bool reactive_led;
extern bool rgb_standby;

void hwinit();
void hardware_timer1_init();
void set_led(volatile uint8_t* PORT,
             uint8_t button_number,
             uint8_t led_pin,
             uint16_t OutputData);
void set_hid_standby_lighting(hid_lights* lights);
void process_buttons(int8_t tt1_report);
void process_button(const volatile uint8_t* PIN,
                    uint8_t button_number,
                    uint8_t input_pin);
void update_tt_transitions(uint8_t reverse_tt);
void process_tt(tt_pins &tt_pin);
void update_lighting(int8_t tt1_report,
                     timer* combo_lights_timer,
                     const Bpm &bpm);
void update_button_lighting(uint16_t led_state,
                            timer* combo_lights_timer);

bool is_pressed(uint16_t button_bits, uint16_t ignore = 0);

// HID functions
void HID_Task();
void ProcessGenericHIDReport(hid_lights led_state);

// button macros to map to bit positions within button_state
#define BUTTON_1 1 << 0
#define BUTTON_2 1 << 1
#define BUTTON_3 1 << 2
#define BUTTON_4 1 << 3
#define BUTTON_5 1 << 4
#define BUTTON_6 1 << 5
#define BUTTON_7 1 << 6
#define BUTTON_8 1 << 7
#define BUTTON_9 1 << 8
#define BUTTON_10 1 << 9
#define BUTTON_11 1 << 10
#define BUTTON_TT_NEG 1 << 11
#define BUTTON_TT_POS 1 << 12
#define EFFECTORS_ALL (BUTTON_8 | BUTTON_9 | BUTTON_10 | BUTTON_11)

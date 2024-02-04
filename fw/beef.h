#pragma once

#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>

#include "Descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include "config.h"
#include "rgb.h"
#include "timer.h"

// HID report structure, for creating and sending HID reports to the
// host PC. This mirrors the layout described to the host in the HID
// report descriptor in descriptors.c.
typedef struct {
  uint8_t  X;
  uint8_t  Y;
  uint16_t Button; // bit-field representing which buttons have been pressed
} USB_JoystickReport_Data_t;

extern bool reactive_led;

void hwinit(void);
void hardware_timer1_init(void);
void set_led(volatile uint8_t* PORT,
             uint8_t button_number,
             uint8_t led_pin,
             uint16_t OutputData);
void set_hid_standby_lighting(hid_lights* lights);
void process_button(volatile uint8_t* PIN,
                    uint8_t button_number,
                    uint8_t input_pin);
void process_tt(volatile uint8_t* PIN,
                uint8_t a_pin,
                uint8_t b_pin,
                int8_t* prev,
                uint16_t* tt_position,
                config current_config);
void update_lighting(int8_t tt1_report,
                     timer* combo_lights_timer,
                     config current_config);
void update_button_lighting(uint16_t led_state,
                            timer* combo_lights_timer,
                            config current_config);
bool is_pressed(uint16_t button_bits);
bool is_pressed_strict(uint16_t button_bits);

// HID functions
void HID_Task(void);
void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);
void ProcessGenericHIDReport(hid_lights led_state);
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize);
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize);

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

// Digital TT
#define BUTTON_12 1 << 11
#define BUTTON_13 1 << 12

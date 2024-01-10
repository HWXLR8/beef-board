// Copyright 2021 Dean Camera (dean [at] fourwalledcubicle [dot] com)
// Copyright 2023 supervaka, HWXLR8

#pragma once

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>

#include "Descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include "rgb.h"
#include "ws2812.h"
#include "timer.h"

// HID report structure, for creating and sending HID reports to the
// host PC. This mirrors the layout described to the host in the HID
// report descriptor in descriptors.c.
typedef struct {
  int8_t  X;
  int8_t  Y;
  int8_t  Z;
  uint16_t Button; // bit-field representing which buttons have been pressed
} USB_JoystickReport_Data_t;

typedef struct {
  volatile uint8_t* DDR;
  volatile uint8_t* PIN;
  volatile uint8_t* PORT;
} hw_pin;

// To bundle each button to its respective pins
// INPUT_PORT : [input pin] : LED_PORT : [LED pin]
typedef struct {
  hw_pin INPUT_PORT;
  uint8_t input_pin;
  hw_pin LED_PORT;
  uint8_t led_pin;
} button_pins;

typedef struct {
  volatile uint8_t* PIN;
  uint8_t a_pin;
  uint8_t b_pin;
  int8_t prev;
  uint16_t tt_position;
} tt_pins;

void hwinit(void);
void hardware_timer1_init(void);
void set_led(volatile uint8_t* PORT,
             uint8_t button_number,
             uint8_t led_pin,
             uint16_t OutputData);
void process_button(volatile uint8_t* PIN,
                    uint8_t button_number,
                    uint8_t input_pin,
		    timer* combo_timer);
void process_tt(volatile uint8_t* PIN,
                uint8_t a_pin,
                uint8_t b_pin,
                int8_t* prev,
                uint16_t* tt_position);
void update_lighting(uint16_t led_data);
bool is_pressed(uint8_t button_bit);

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
#define BUTTON_1 0
#define BUTTON_2 1
#define BUTTON_3 2
#define BUTTON_4 3
#define BUTTON_5 4
#define BUTTON_6 5
#define BUTTON_7 6
#define BUTTON_8 7
#define BUTTON_9 8
#define BUTTON_10 9
#define BUTTON_11 10

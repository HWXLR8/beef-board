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

// HID report structure, for creating and sending HID reports to the
// host PC. This mirrors the layout described to the host in the HID
// report descriptor in descriptors.c.
typedef struct {
  int8_t  X;
  int8_t  Y;
  int8_t  Z;
  uint16_t Button; // bit-field representing which buttons have been pressed
} USB_JoystickReport_Data_t;

// The OUT report structure
typedef struct {
  uint16_t Lights; // 16 bits for 16 lights
  uint8_t  Command;
  uint8_t  Data;
} Output_t;

// To bundle each button to its respective pins
typedef struct {
	volatile uint8_t* pin;
	volatile uint8_t* port;
	uint8_t button_number;
	uint8_t input_pin;
	uint8_t led_pin;
} button_pins;

void SetupHardware(void);
void HID_Task(void);
void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);
void ProcessGenericHIDReport(Output_t* ReportData);
void set_led(volatile uint8_t* PORT, 
						 uint8_t button_number, 
						 uint8_t led_pin, 
						 uint16_t OutputData);
void process_button(volatile uint8_t* PIN, 
										volatile uint8_t* PORT, 
										uint8_t button_number, 
										uint8_t input_pin, 
										uint8_t led_pin);
void process_tt(volatile uint8_t* PINx, 
								int8_t* prev, 
								int8_t* tt_position);
void Lights_SetState(uint16_t OutputData);
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

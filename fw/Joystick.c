/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the Joystick demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Joystick.h"


/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Joystick_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Joystick,
				.ReportINEndpoint             =
					{
						.Address              = JOYSTICK_IN_EPADDR,
						.Size                 = JOYSTICK_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevJoystickHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevJoystickHIDReportBuffer),
			},
	};
	
// not sure if using global variables in this manner will lead to problems down the line
int8_t turntablePosition = 0;
// bit-field for the buttons
// first 11 bits map to the state of each button
uint16_t button = 0;
// struct from usbemani-legacy
Settings_Lights_t *Lights;
// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding button is held
bool reactiveLightingMode = true;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	SetupHardware();
	USB_Init();
	// pin setup

	// for turntable
	// we will set pins F0 and F1 as inputs to the
	// photo interrupters
	// idk what F2-F7 are used for atm
	DDRF  &= 0b11111100;
	PORTF |= 0b00000011;

	// for buttons
	// buttons to pins:
	// <name>   : <input pin> : <LED pin>
	// BUTTON 1 : B0 : B1
	// BUTTON 2 : B2 : B3
	// BUTTON 3 : B4 : B5
	// BUTTON 4 : B6 : B7
	// BUTTON 5 : D0 : D1
	// BUTTON 6 : D2 : D3
	// BUTTON 7 : D4 : D5
	// START    : D6 : D7
	// VEFX     : C0 : C1
	// EFFECT   : C2 : C3
	// AUX      : C4 : C5

	// leaving this comment since I am noob at working with bit-fields
	// using &= and |= for DDRC and PORTC should achieve an assignment of 
	// DDR? = 0bXXYYYYYY where the XX bits are unchanged from whatever they previously were
	DDRB  = 0b10101010;
	DDRD  = 0b10101010;
	DDRC &= 0b11101010;
	DDRC |= 0b00101010;

	PORTB  = 0b01010101;
	PORTD  = 0b01010101;
	PORTC |= 0b00010101;
	PORTC &= 0b11010101;


	int8_t prev = -1;
	int8_t curr = -1;


	GlobalInterruptEnable();

	for (;;)
	{
		HID_Device_USBTask(&Joystick_HID_Interface);
		// our HID_Task() only takes care of processing HID reports
		HID_Task();
		USB_USBTask();

		/*---------------------turntable logic---------------------*/
		// curr is binary number ab
		// where a is the signal of F0
		// and   b is the signal of F1
		// e.g. when F0 == 1 and F1 == 0, then curr == 0b10
		int8_t a = PINF & (1 << 0) ? 1 : 0;
		int8_t b = PINF & (1 << 1) ? 1 : 0;
		curr = (a << 1) | b;

		if (prev == 3 && curr == 1 || prev == 1 && curr == 0 ||
				prev == 0 && curr == 2 || prev == 2 && curr == 3)
		{
			turntablePosition++;
		}
		else if (prev == 1 && curr == 3 || prev == 0 && curr == 1 ||
						 prev == 2 && curr == 0 || prev == 3 && curr == 2)
		{
			turntablePosition--;
		}
		prev = curr;

		// BUTTON 1 : B0 : B1
		if (~PINB & (1 << 0))
		{
			button |= (1 << 0);
			if (reactiveLightingMode) PORTB |= (1 << 1);
		}
		else
		{
			button &= ~(1 << 0);
			if (reactiveLightingMode) PORTB &= ~(1 << 1);
		}

		// BUTTON 2 : B2 : B3
		if (~PINB & (1 << 2))
		{
			button |= (1 << 1);
			if (reactiveLightingMode) PORTB |= (1 << 3);
		}
		else
		{
			button &= ~(1 << 1);
			if (reactiveLightingMode) PORTB &= ~(1 << 3);
		}

		// BUTTON 3 : B4 : B5
		if (~PINB & (1 << 4))
		{
			button |= (1 << 2);
			if (reactiveLightingMode) PORTB |= (1 << 5);
		}
		else
		{
			button &= ~(1 << 2);
			if (reactiveLightingMode) PORTB &= ~(1 << 5);
		}

		// BUTTON 4 : B6 : B7
		if (~PINB & (1 << 6))
		{
			button |= (1 << 3);
			if (reactiveLightingMode) PORTB |= (1 << 7);
		}
		else
		{
			button &= ~(1 << 3);
			if (reactiveLightingMode) PORTB &= ~(1 << 7);
		}

		// BUTTON 5 : D0 : D1
		if (~PIND & (1 << 0))
		{
			button |= (1 << 4);
			if (reactiveLightingMode) PORTD |= (1 << 1);
		}
		else
		{
			button &= ~(1 << 4);
			if (reactiveLightingMode) PORTD &= ~(1 << 1);
		}

		// BUTTON 6 : D2 : D3
		if (~PIND & (1 << 2))
		{
			button |= (1 << 5);
			if (reactiveLightingMode) PORTD |= (1 << 3);
		}
		else
		{
			button &= ~(1 << 5);
			if (reactiveLightingMode) PORTD &= ~(1 << 3);
		}

		// BUTTON 7 : D4 : D5
		if (~PIND & (1 << 4))
		{
			button |= (1 << 6);
			if (reactiveLightingMode) PORTD |= (1 << 5);
		}
		else
		{
			button &= ~(1 << 6);
			if (reactiveLightingMode) PORTD &= ~(1 << 5);
		}

		// START : D6 : D7
		if (~PIND & (1 << 6))
		{
			button |= (1 << 7);
			if (reactiveLightingMode) PORTD |= (1 << 7);
		}
		else
		{
			button &= ~(1 << 7);
			if (reactiveLightingMode) PORTD &= ~(1 << 7);
		}

		// VEFX : C0 : C1
		if (~PINC & (1 << 0))
		{
			button |= (1 << 8);
			if (reactiveLightingMode) PORTC |= (1 << 1);
		}
		else
		{
			button &= ~(1 << 8);
			if (reactiveLightingMode) PORTC &= ~(1 << 1);
		}

		// EFFECT : C2 : C3
		if (~PINC & (1 << 2))
		{
			button |= (1 << 9);
			if (reactiveLightingMode) PORTC |= (1 << 3);
		}
		else
		{
			button &= ~(1 << 9);
			if (reactiveLightingMode) PORTC &= ~(1 << 3);
		}

		// AUX : C4 : C5
		if (~PINC & (1 << 4))
		{
			button |= (1 << 10);
			if (reactiveLightingMode) PORTC |= (1 << 5);
		}
		else
		{
			button &= ~(1 << 10);
			if (reactiveLightingMode) PORTC &= ~(1 << 5);
		}
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	USB_Init();
	reactiveLightingMode = true;
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void){}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void){}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	// ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);

	//USB_Device_EnableSOFEvents();
	
	// not sure if this does anything
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
}


void EVENT_USB_Device_ControlRequest(void)
{
	/* Handle HID Class specific requests */
	switch (USB_ControlRequest.bRequest)
	{
		case HID_REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Output_t LightsData;

				Endpoint_ClearSETUP();

				/* Read the report data from the control endpoint */
				Endpoint_Read_Control_Stream_LE(&LightsData, sizeof(LightsData));
				Endpoint_ClearIN();

				ProcessGenericHIDReport(&LightsData);
			}

			break;
	}
}

/** Function to process the last received report from the host.
 *
 *  \param[in] DataArray  Pointer to a buffer where the last received report has been stored
 */
void ProcessGenericHIDReport(Output_t* ReportData)
{
	/*
		This is where you need to process reports sent from the host to the device. This
		function is called each time the host has sent a new report. DataArray is an array
		holding the report sent from the host.
	*/
	reactiveLightingMode = false;

	// We also need to forward the lighting data over.
	Lights_SetState(ReportData->Lights);
}

void HID_Task(void)
{
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

	/* Check to see if a packet has been sent from the host */
	if (Endpoint_IsOUTReceived())
	{
		/* Check to see if the packet contains data */
		if (Endpoint_IsReadWriteAllowed())
		{
			/* Create a temporary buffer to hold the read in report from the host */
			Output_t LightsData;

			/* Read Generic Report Data */
			Endpoint_Read_Stream_LE(&LightsData, sizeof(LightsData), NULL);

			/* Process Generic Report Data */
			ProcessGenericHIDReport(&LightsData);
		}

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearOUT();
	}
}


/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

	JoystickReport->X = turntablePosition;
	JoystickReport->Button = button;

	*ReportSize = sizeof(USB_JoystickReport_Data_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

void Lights_SetState(uint16_t OutputData) {
  // buttons to pins:
	// <name>   : <input pin> : <LED pin>
	// BUTTON 1 : B0 : B1
  if (OutputData & (1 << 0)) {
    PORTB |= (1 << 1);
  } else {
    PORTB &= ~(1 << 1);
  }
	// BUTTON 2 : B2 : B3
  if (OutputData & (1 << 1)) {
    PORTB |= (1 << 3);
  } else {
    PORTB &= ~(1 << 3);
  }
	// BUTTON 3 : B4 : B5
  if (OutputData & (1 << 2)) {
    PORTB |= (1 << 5);
  } else {
    PORTB &= ~(1 << 5);
  }
	// BUTTON 4 : B6 : B7
  if (OutputData & (1 << 3)) {
    PORTB |= (1 << 7);
  } else {
    PORTB &= ~(1 << 7);
  }
	// BUTTON 5 : D0 : D1
  if (OutputData & (1 << 4)) {
    PORTD |= (1 << 1);
  } else {
    PORTD &= ~(1 << 1);
  }
	// BUTTON 6 : D2 : D3
  if (OutputData & (1 << 5)) {
    PORTD |= (1 << 3);
  } else {
    PORTD &= ~(1 << 3);
  }
	// BUTTON 7 : D4 : D5
  if (OutputData & (1 << 6)) {
    PORTD |= (1 << 5);
  } else {
    PORTD &= ~(1 << 5);
  }
	// START    : D6 : D7
  if (OutputData & (1 << 7)) {
    PORTD |= (1 << 7);
  } else {
    PORTD &= ~(1 << 7);
  }
	// VEFX     : C0 : C1
  if (OutputData & (1 << 8)) {
    PORTC |= (1 << 1);
  } else {
    PORTC &= ~(1 << 1);
  }
	// EFFECT   : C2 : C3
  if (OutputData & (1 << 9)) {
    PORTC |= (1 << 3);
  } else {
    PORTC &= ~(1 << 3);
  }
	// AUX      : C4 : C5
  if (OutputData & (1 << 10)) {
    PORTC |= (1 << 5);
  } else {
    PORTC &= ~(1 << 5);
  }
}
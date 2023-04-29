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
 *  the demo and is responsible for the initial application hardware
 * configuration.
 */

#include "Joystick.h"

/** Buffer to hold the previously generated HID report, for comparison purposes
 * inside the HID class driver. */
static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This
 * structure is passed to all HID Class driver functions, so that multiple
 * instances of the same class within a device can be differentiated from one
 * another.
 */
USB_ClassInfo_HID_Device_t Joystick_HID_Interface = {
    .Config =
        {
            .InterfaceNumber = INTERFACE_ID_Joystick,
            .ReportINEndpoint =
                {
                    .Address = JOYSTICK_EPADDR,
                    .Size = JOYSTICK_EPSIZE,
                    .Banks = 1,
                },
            .PrevReportINBuffer = PrevJoystickHIDReportBuffer,
            .PrevReportINBufferSize = sizeof(PrevJoystickHIDReportBuffer),
        },
};

/** Main program entry point. This routine contains the overall program flow,
 * including initial setup of all components and the main program loop.
 */
int8_t turntablePosition = 0;
int main(void) {
  SetupHardware();

  // pin setup
  // we will set port C and port F as inputs to the
  // photo interrupters
  // specifically pins C6 and F7
  DDRC = 0;
  DDRF = 0;
  PORTC = 0xFF;
  PORTF = 0xFF;

  // this is just to turn off the LEDs, they were annoying
  DDRD = 0xFF;
  DDRB = 0xFF;
  PORTD = 0xFF;
  PORTB = 0xFF;

  int prev = -1;
  int curr = -1;
  GlobalInterruptEnable();

  for (;;) {
    HID_Device_USBTask(&Joystick_HID_Interface);
    USB_USBTask();

    /*---------------------turntable logic---------------------*/
    // curr is binary value ab
    // where a is the signal of C6
    // and   b is the signal of F7
    // e.g. when C6 == 1 and F7 == 0, then curr == 0b10
    int a = PINC & (1 << 6) ? 1 : 0;
    int b = PINF & (1 << 7) ? 1 : 0;
    curr = (a << 1) | b;

    if (prev == 3 && curr == 1 || prev == 1 && curr == 0 ||
        prev == 0 && curr == 2 || prev == 2 && curr == 3) {
      turntablePosition++;
    } else if (prev == 1 && curr == 3 || prev == 0 && curr == 1 ||
               prev == 2 && curr == 0 || prev == 3 && curr == 2) {
      turntablePosition--;
    } else {
      // silence both ports when there is no motion
      // we should reach this branch when (prev == curr)
      ;
    }
    prev = curr;

    /*---------------------button logic---------------------*/
  }
}

/** HID class driver callback function for the creation of HID reports to the
 * host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface
 * configuration structure being referenced \param[in,out] ReportID    Report ID
 * requested by the host if non-zero, otherwise callback should set to the
 * generated report ID \param[in]     ReportType  Type of the report to create,
 * either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature \param[out] ReportData
 * Pointer to a buffer where the created report should be stored \param[out]
 * ReportSize  Number of bytes written in the report (or zero if no report is to
 * be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let
 * the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(
    USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, uint8_t* const ReportID,
    const uint8_t ReportType, void* ReportData, uint16_t* const ReportSize) {
  USB_JoystickReport_Data_t* JoystickReport =
      (USB_JoystickReport_Data_t*)ReportData;

  // button press
  // if (!(PINB & _BV(PB0))) {
  //   JoystickReport->Button |= (1 << 0);
  // }

  // turntable movement
  JoystickReport->X = turntablePosition;
  // JoystickReport->Button = ;

  *ReportSize = sizeof(USB_JoystickReport_Data_t);
  return false;
}

/** Configures the board hardware and chip peripherals for the demo's
 * functionality. */
void SetupHardware(void) {
#if (ARCH == ARCH_AVR8)
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
  /* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the
   * CPU core to run from it */
  XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
  XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

  /* Start the 32MHz internal RC oscillator and start the DFLL to increase it to
   * 48MHz using the USB SOF as a reference */
  XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
  XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

  PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

  /* Hardware Initialization */
  USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) {}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void) {}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void) {
  bool ConfigSuccess = true;

  ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);

  USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void) {
  HID_Device_ProcessControlRequest(&Joystick_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void) {
  HID_Device_MillisecondElapsed(&Joystick_HID_Interface);
}

/** HID class driver callback function for the processing of HID reports from
 * the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface
 * configuration structure being referenced \param[in] ReportID    Report ID of
 * the received report from the host \param[in] ReportType  The type of report
 * that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has
 * been stored \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(
    USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, const uint8_t ReportID,
    const uint8_t ReportType, const void* ReportData,
    const uint16_t ReportSize) {
  // Unused (but mandatory for the HID class driver) in this demo, since there
  // are no Host->Device reports
}

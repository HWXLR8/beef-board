#pragma once

#include "Descriptors.h"

template <typename T, uint8_t InterfaceNumber, uint8_t Endpoint>
struct HidReport {
  // buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver.
  uint8_t PrevHIDReportBuffer[sizeof(T)]{};

  USB_ClassInfo_HID_Device_t HID_Interface = {
    .Config = {
      .InterfaceNumber = InterfaceNumber,
      .ReportINEndpoint = {
        .Address = Endpoint,
        .Size = HID_EPSIZE,
        .Banks = 1,
      },
      .PrevReportINBuffer = PrevHIDReportBuffer,
      .PrevReportINBufferSize = sizeof(PrevHIDReportBuffer),
    },
  };
};

// HID functions
template<typename T>
void HID_Task(T* led_state) {
  Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

  // check if a packet has been sent from the host
  if (Endpoint_IsOUTReceived()) {
    // check if packet contains data
    if (Endpoint_IsReadWriteAllowed()) {// read generic report data
      Endpoint_Read_Stream_LE(led_state, sizeof(*led_state), nullptr);

      reactive_led = false;
      rgb_standby = false;
      timer_arm(&hid_lights_expiry_timer, 1000);
    }

    // finalize the stream transfer to send the last packet
    Endpoint_ClearOUT();
  }
}

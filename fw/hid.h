#pragma once

#include "Descriptors.h"
#include "timer.h"

// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
extern bool reactive_led;
extern bool rgb_standby;
extern timer hid_lights_expiry_timer;

namespace Beef {
  struct USB_KeyboardReport_Data_t {
    uint8_t KeyCode[KEYBOARD_KEYS];
  } ATTR_PACKED;

  struct USB_MouseReport_Data_t {
    int8_t X;
    int8_t Y;
  } ATTR_PACKED;
}

template <typename T, uint8_t interface, uint8_t endpoint>
struct HidReport {
  // buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver.
  uint8_t PrevHIDReportBuffer[sizeof(T)]{};

  USB_ClassInfo_HID_Device_t HID_Interface = {
    .Config = {
      .InterfaceNumber = interface,
      .ReportINEndpoint = {
        .Address = endpoint,
        .Size = HID_EPSIZE,
        .Type = EP_TYPE_INTERRUPT,
        .Banks = 1,
      },
      .PrevReportINBuffer = PrevHIDReportBuffer,
      .PrevReportINBufferSize = sizeof(PrevHIDReportBuffer),
    },
    .State = {
      .UsingReportProtocol = true,
      .IdleCount = 500
    }
  };
};

// HID functions
template<typename T>
void HID_Task(T &led_state, const uint8_t endpoint) {
  Endpoint_SelectEndpoint(endpoint);

  // check if a packet has been sent from the host
  if (Endpoint_IsOUTReceived()) {
    // check if packet contains data
    if (Endpoint_IsReadWriteAllowed()) { // read generic report data
      Endpoint_Read_Stream_LE(&led_state, sizeof(T), nullptr);

      timer_arm(&hid_lights_expiry_timer, 1000);
    }

    // finalize the stream transfer to send the last packet
    Endpoint_ClearOUT();
  }
}

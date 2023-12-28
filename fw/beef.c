// Copyright 2021 Dean Camera (dean [at] fourwalledcubicle [dot] com)
// Copyright 2023 supervaka, HWXLR8

#include "beef.h"
#include "config.h"
#include "analog_turntable.h"
#include "tt_rgb_manager.h"

// buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver.
static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

// LUFA HID Class driver interface configuration and state
// information. This structure is passed to all HID Class driver
// functions, so that multiple instances of the same class within a
// device can be differentiated from one another.
USB_ClassInfo_HID_Device_t Joystick_HID_Interface = {
  .Config = {
    .InterfaceNumber = INTERFACE_ID_Joystick,
    .ReportINEndpoint = {
      .Address = JOYSTICK_IN_EPADDR,
      .Size = JOYSTICK_EPSIZE,
      .Banks = 1,
    },
    .PrevReportINBuffer = PrevJoystickHIDReportBuffer,
    .PrevReportINBufferSize = sizeof(PrevJoystickHIDReportBuffer),
  },
};

// bit-field storing button state. bits 0-10 map to buttons 1-11
uint16_t button_state = 0;
uint16_t led_state_from_hid_report = 0;
// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
bool reactive_led = true;
// temporary hack? because set_led() needs access to buttons[]
button_pins* buttons_ptr;

timer hid_lights_expiry_timer;

ISR(TIMER1_COMPA_vect) {
  milliseconds++;
}

int main(void) {
  SetupHardware();
  USB_Init();

  timer my_timer;
  timer_init(&my_timer);
  timer_arm(&my_timer, 500);

  timer_init(&led_timer);

  timer_init(&spin_timer);
  timer_arm(&spin_timer, 100);

  timer_init(&flash_timer);
  timer_arm(&flash_timer, 500);

  timer_init(&hid_lights_expiry_timer);
  timer_arm(&hid_lights_expiry_timer, 5000);

  analog_turntable tt1;
  analog_turntable_init(&tt1, 4, 200, true);

  // tt_x DATA lines wired to F0/F1
  DDRF  &= 0b11111100;
  PORTF |= 0b00000011;

  button_pins buttons[] = CONFIG_ALL_HW_PIN;
  buttons_ptr = buttons;

  for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); ++i) {
    CONFIG_DDR_INPUT(buttons[i].INPUT_PORT.DDR, buttons[i].input_pin);
    CONFIG_DDR_LED(buttons[i].LED_PORT.DDR, buttons[i].led_pin);

    CONFIG_PORT_INPUT(buttons[i].INPUT_PORT.PORT, buttons[i].input_pin);
    CONFIG_PORT_LED(buttons[i].LED_PORT.PORT, buttons[i].led_pin);
  }

  GlobalInterruptEnable();

  while (1) {
    HID_Device_USBTask(&Joystick_HID_Interface);
    HID_Task();
    USB_USBTask();

    if (!timer_is_armed(&hid_lights_expiry_timer) ||
        timer_check_if_expired_reset(&hid_lights_expiry_timer)) {
      reactive_led = true;
    }

    int8_t tt1_report = analog_turntable_poll(&tt1, tt_x.tt_position);
    tt_rgb_manager_update(tt1_report);
    light_bar_update();

    if (reactive_led) {
      update_lighting(button_state);
    } else {
      update_lighting(led_state_from_hid_report);
    }

    process_tt(tt_x.PIN,
	       tt_x.a_pin,
	       tt_x.b_pin,
	       &tt_x.prev,
	       &tt_x.tt_position);
    // process_tt(tt_y.PIN, tt_y.a_pin, tt_y.b_pin, &tt_y.prev, &tt_y.tt_position);

    for (int i = 0; i < 11; ++i) {
      process_button(buttons[i].INPUT_PORT.PIN,
                     buttons[i].LED_PORT.PORT,
                     i,
                     buttons[i].input_pin,
                     buttons[i].led_pin);
    }
  }
}

// this refers to the hardware timer peripheral
// unrelated to the timer class in timer.h
void hardware_timer1_init(void) {
  // set up Timer1 in CTC (Clear Timer on Compare Match) mode
  TCCR1B |= (1 << WGM12);

  // set the value to compare to
  // assuming a 16MHz clock and a prescaler of 64, this will give us a 1ms tick
  // (16000000 / (64 * 1000)) - 1 = 249
  OCR1A = 249;

  // enable the compare match interrupt
  TIMSK1 |= (1 << OCIE1A);

  // enable global interrupts
  sei();

  // start the timer with a prescaler of 64
  TCCR1B |= (1 << CS10) | (1 << CS11);
}

// configure board hardware and chip peripherals
void SetupHardware(void) {
  // disable watchdog if enabled by bootloader/fuses
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // disable clock division
  clock_prescale_set(clock_div_1);

  // hardware init
  USB_Init();
  hardware_timer1_init();
  reactive_led = true;
}

// event handler for USB connection event
void EVENT_USB_Device_Connect(void){}

// event handler for USB disconnection event
void EVENT_USB_Device_Disconnect(void){}

// event handler for USB config change event
void EVENT_USB_Device_ConfigurationChanged(void) {
  bool ConfigSuccess = true;

  // setup HID report endpoints
  ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
  ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
}

void EVENT_USB_Device_ControlRequest(void) {
  // handle HID Class specific requests
  switch (USB_ControlRequest.bRequest) {
    case HID_REQ_SetReport:
      if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
        uint16_t led_state;

        Endpoint_ClearSETUP();

        // read report data from the control endpoint
        Endpoint_Read_Control_Stream_LE(&led_state, sizeof(led_state));
        Endpoint_ClearIN();

        ProcessGenericHIDReport(led_state);
      }
    break;
  }
}

// process last received report from the host.
void ProcessGenericHIDReport(uint16_t led_state) {
  reactive_led = false;
  timer_arm(&hid_lights_expiry_timer, 5000);

  // update the lighting data
  led_state_from_hid_report = led_state;
}

void HID_Task(void) {
  Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

  // check if a packet has been sent from the host
  if (Endpoint_IsOUTReceived()) {
    // check if packet contains data
    if (Endpoint_IsReadWriteAllowed()) {
      //temp buffer to hold the read in report from the host
      uint16_t led_state;

      // read generic report data
      Endpoint_Read_Stream_LE(&led_state, sizeof(led_state), NULL);

      // process generic report data
      ProcessGenericHIDReport(led_state);
    }

    // finalize the stream transfer to send the last packet
    Endpoint_ClearOUT();
  }
}


bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize) {
  USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

  JoystickReport->X = ((int8_t)(tt_x.tt_position / TT_RATIO)) - 128;
  // JoystickReport->Y = tt_y.tt_position;
  JoystickReport->Button = button_state;

  *ReportSize = sizeof(USB_JoystickReport_Data_t);
  return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize) {
  // Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

void set_led(volatile uint8_t* PORT,
             uint8_t button_number,
             uint8_t led_pin,
             uint16_t led_state) {
  if (led_state & (1 << button_number)) {
    *PORT |= (1 << led_pin);
  } else {
    *PORT &= ~(1 << led_pin);
  }
}

void process_button(volatile uint8_t* PIN,
                    volatile uint8_t* PORT,
                    uint8_t button_number,
                    uint8_t input_pin,
                    uint8_t led_pin) {
  if (~*PIN & (1 << input_pin)) {
    button_state |= (1 << button_number);
  } else {
    button_state &= ~(1 << button_number);
  }
}

void process_tt(volatile uint8_t* PIN,
                uint8_t a_pin,
                uint8_t b_pin,
                int8_t* prev,
                uint16_t* tt_position) {
  // tt logic
  // example where tt_x wired to F0/F1:
  // curr is binary number ab
  // where a is the signal of F0
  // and b is the signal of F1
  // therefore when F0 == 1 and F1 == 0, then curr == 0b10
  int8_t a = *PIN & (1 << a_pin) ? 1 : 0;
  int8_t b = *PIN & (1 << b_pin) ? 1 : 0;
  int8_t curr = (a << 1) | b;

  if (*prev == 3 && curr == 1 ||
      *prev == 1 && curr == 0 ||
      *prev == 0 && curr == 2 ||
      *prev == 2 && curr == 3) {
    (*tt_position)--;
  } else if (*prev == 1 && curr == 3 ||
             *prev == 0 && curr == 1 ||
             *prev == 2 && curr == 0 ||
             *prev == 3 && curr == 2) {
    (*tt_position)++;
  }
  *tt_position = *tt_position % (256 * TT_RATIO);
  *prev = curr;
}

void update_lighting(uint16_t led_state) {
  for (int i = 0; i < 11; ++i) {
    set_led(buttons_ptr[i].LED_PORT.PORT,
            i,
            buttons_ptr[i].led_pin,
            led_state);
  }
}

// Copyright 2021 Dean Camera (dean [at] fourwalledcubicle [dot] com)
// Copyright 2023 supervaka, HWXLR8

#include <avr/eeprom.h>

#include "analog_turntable.h"
#include "beef.h"
#include "rgb_manager.h"

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
hid_lights led_state_from_hid_report;

// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
bool reactive_led = true;
// temporary hack? because set_led() needs access to buttons[]
button_pins* buttons_ptr;

#define HID_LIGHTS_EXPIRY_TIME 1000
timer hid_lights_expiry_timer;

// this assumes that the pins to the 2 DATA lines are on the same port
// will need to refactor if this is not the case
// e.g. if a_pin is on F0 and b_pin is on D0
// PIN : [a_pin] : [b_pin] : [prev] : [tt_position]
tt_pins tt_x = { &PINF, 0, 1, -1, 0 };
// tt_pins tt_y = { &PIN?, ?, ?, -1, 0 };

config current_config;

typedef struct {
  uint16_t button_combo;
  void (*config_set)(config*);
} combo;

combo button_combos[NUM_OF_COMBOS] = {
  {
    button_combo: REVERSE_TT_COMBO,
    config_set: toggle_reverse_tt
  },
  {
    button_combo: TT_EFFECTS_COMBO,
    config_set: cycle_tt_effects
  },
  {
    button_combo: TT_DEADZONE_INCR_COMBO,
    config_set: increase_deadzone
  },
  {
    button_combo: TT_DEADZONE_DECR_COMBO,
    config_set: decrease_deadzone
  },
  {
    button_combo: BAR_EFFECTS_COMBO,
    config_set: cycle_bar_effects
  },
  {
    button_combo: DISABLE_LED_COMBO,
    config_set: toggle_disable_led
  },
};

ISR(TIMER1_COMPA_vect) {
  milliseconds++;
}

int main(void) {
  hwinit();

  config_init(&current_config);

  timer_init(&hid_lights_expiry_timer);

  timer combo_timer;
  timer combo_lights_timer;
  timer_init(&combo_timer);
  timer_init(&combo_lights_timer);

  analog_turntable_init(&tt1, current_config.tt_deadzone, 200, true);

  RgbManager::Turntable::init();
  RgbManager::Bar::init();

  // tt_x DATA lines wired to F0/F1
  DDRF  &= 0b11111100;
  PORTF |= 0b00000011;

  button_pins buttons[] = CONFIG_ALL_HW_PIN;
  buttons_ptr = buttons;

  for (int i = 0; i < int(sizeof(buttons)/sizeof(buttons[0])); ++i) {
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

    if (timer_check_if_expired_reset(&hid_lights_expiry_timer)) {
      reactive_led = true;
      set_hid_standby_lighting(&led_state_from_hid_report);
    }

    int8_t tt1_report = analog_turntable_poll(&tt1, tt_x.tt_position);

    for (int i = 0; i < BUTTONS; ++i) {
      process_button(buttons[i].INPUT_PORT.PIN,
                     i,
                     buttons[i].input_pin);
    }
    process_combos(&current_config,
                   &combo_timer,
                   &combo_lights_timer);

    process_tt(tt_x.PIN,
               tt_x.a_pin,
               tt_x.b_pin,
               &tt_x.prev,
               &tt_x.tt_position,
               current_config);

    update_lighting(tt1_report,
                    &combo_lights_timer,
                    current_config);
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
void hwinit(void) {
  // disable watchdog if enabled by bootloader/fuses
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // disable clock division
  clock_prescale_set(clock_div_1);

  // hardware init
  USB_Init();
  hardware_timer1_init();

  reactive_led = true;
  set_hid_standby_lighting(&led_state_from_hid_report);
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

void EVENT_USB_Device_ControlRequest(void){}

// process last received report from the host.
void ProcessGenericHIDReport(hid_lights led_state) {
  reactive_led = false;
  timer_arm(&hid_lights_expiry_timer, HID_LIGHTS_EXPIRY_TIME);

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
      hid_lights led_state;

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

  JoystickReport->X = tt_x.tt_position / BEEF_TT_RATIO;
  // JoystickReport->Y = tt_y.tt_position;
  JoystickReport->Button = button_state;

  *ReportSize = sizeof(USB_JoystickReport_Data_t);
  return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize){}

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

void set_hid_standby_lighting(hid_lights* lights) {
  lights->buttons = 0;
  lights->tt_lights = { 255, 255, 255 };
  lights->bar_lights = { 255, 255, 255 };
}

void process_button(volatile uint8_t* PIN,
                    uint8_t button_number,
                    uint8_t input_pin) {
  if (~*PIN & (1 << input_pin)) {
    button_state |= (1 << button_number);
  } else {
    button_state &= ~(1 << button_number);
  }
}

void process_combos(config* current_config,
                    timer* combo_timer,
                    timer* combo_lights_timer) {
  static bool ignore_combo = false;
  bool combo_pressed = false;

  for (int i = 0; i < NUM_OF_COMBOS; ++i) {
    if (is_pressed_strict(button_combos[i].button_combo)) {
      combo_pressed = true;
      if (ignore_combo) {
        return;
      }

      // arm timer if not already armed
      if (!timer_is_armed(combo_timer)) {
        timer_arm(combo_timer, 1000);
      }

      if (timer_check_if_expired_reset(combo_timer)) {
        button_combos[i].config_set(current_config);
        timer_arm(combo_lights_timer, CONFIG_CHANGE_NOTIFY_TIME);
        ignore_combo = true;
      }

      return;
    }
  }

  if (!combo_pressed) {
    ignore_combo = false;
    timer_init(combo_timer);
    timer_init(combo_lights_timer);
    timer_init(&RgbManager::Turntable::combo_timer);
  }
}

void process_tt(volatile uint8_t* PIN,
                uint8_t a_pin,
                uint8_t b_pin,
                int8_t* prev,
                uint16_t* tt_position,
                config current_config) {
  // tt logic
  // example where tt_x wired to F0/F1:
  // curr is binary number ab
  // where a is the signal of F0
  // and b is the signal of F1
  // therefore when F0 == 1 and F1 == 0, then curr == 0b10
  int8_t a = *PIN & (1 << a_pin) ? 1 : 0;
  int8_t b = *PIN & (1 << b_pin) ? 1 : 0;
  int8_t curr = (a << 1) | b;

  int8_t direction = current_config.reverse_tt ? -1 : 1;

  if ((*prev == 3 && curr == 1) ||
      (*prev == 1 && curr == 0) ||
      (*prev == 0 && curr == 2) ||
      (*prev == 2 && curr == 3)) {
    *tt_position -= direction;
  } else if ((*prev == 1 && curr == 3) ||
             (*prev == 0 && curr == 1) ||
             (*prev == 2 && curr == 0) ||
             (*prev == 3 && curr == 2)) {
    *tt_position += direction;
  }
  *tt_position %= 256 * BEEF_TT_RATIO;
  *prev = curr;
}

void update_lighting(int8_t tt1_report,
                     timer* combo_lights_timer,
                     config current_config) {
  if (reactive_led) {
    update_button_lighting(button_state,
                           combo_lights_timer,
                           current_config);
  } else {
    update_button_lighting(led_state_from_hid_report.buttons,
                           combo_lights_timer,
                           current_config);
  }

  if (!current_config.disable_led) {
    // update lighting on a timer to reduce the number of
    // computationally expensive calls to FastLED.show();
    EVERY_N_MILLIS(1000 / BEEF_LED_REFRESH) {
      RgbManager::Turntable::update(tt1_report,
                                    led_state_from_hid_report.tt_lights,
                                    current_config.tt_effect);
      RgbManager::Bar::update(led_state_from_hid_report.bar_lights,
                              current_config.bar_effect);

      FastLED.show();
    }
  }
}

void update_button_lighting(uint16_t led_state,
                            timer* combo_lights_timer,
			    config current_config) {
  if (current_config.disable_led) {
    led_state = 0;
  }

  if (timer_is_active(combo_lights_timer)) {
    // Temporarily black out button LEDs to notify a mode change
    led_state = 0;
  }

  for (int i = 0; i < BUTTONS; ++i) {
    set_led(buttons_ptr[i].LED_PORT.PORT,
            i,
            buttons_ptr[i].led_pin,
            led_state);
  }
}

bool is_pressed(uint16_t button_bits) {
  return button_state & button_bits;
}

bool is_pressed_strict(uint16_t button_bits) {
  return button_state == button_bits;
}

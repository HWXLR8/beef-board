#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>
#include "Descriptors.h"

#include "analog_turntable.h"
#include "beef.h"
#include "combo.h"
#include "debounce.h"
#include "fastled_shim.h"
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
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;
hid_lights led_state_from_hid_report;

// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
bool reactive_led = true;
bool rgb_standby = true;
int8_t tt_transitions[4][4];
button_pins buttons[] = CONFIG_ALL_HW_PIN;

#define HID_LIGHTS_EXPIRY_TIME 1000
timer hid_lights_expiry_timer;

// this assumes that the pins to the 2 DATA lines are on the same port
// will need to refactor if this is not the case
// e.g. if a_pin is on F0 and b_pin is on D0
// PIN : [a_pin] : [b_pin] : [prev] : [tt_position]
tt_pins tt_x = { &PINF, 0, 1, -1, 0 };

config current_config;

ISR(TIMER1_COMPA_vect) {
  milliseconds++;
}

void debounce(DebounceState* debounce, uint16_t mask) {
  button_state =
    (button_state & ~mask) |
    (debounce->debounce(button_state & mask));
}

int main() {
  hwinit();

  config_init(&current_config);

  timer_init(&hid_lights_expiry_timer);

  timer combo_timer;
  timer combo_lights_timer;
  timer_init(&combo_timer);
  timer_init(&combo_lights_timer);

  // Centre tt_position to prevent misfire on first input
  tt_x.tt_position = 128 * current_config.tt_ratio;
  analog_turntable_init(&tt1, current_config.tt_deadzone, 200, true);

  Bpm bpm(LIGHT_BAR_LEDS);

  RgbManager::init();

  // tt_x DATA lines wired to F0/F1
  DDRF  &= 0b11111100;
  PORTF |= 0b00000011;

  for (auto &button : buttons) {
    CONFIG_DDR_INPUT(button.INPUT_PORT.DDR, button.input_pin);
    CONFIG_DDR_LED(button.LED_PORT.DDR, button.led_pin);

    CONFIG_PORT_INPUT(button.INPUT_PORT.PORT, button.input_pin);
    CONFIG_PORT_LED(button.LED_PORT.PORT, button.led_pin);
  }

  GlobalInterruptEnable();

  while (true) {
    HID_Device_USBTask(&Joystick_HID_Interface);
    HID_Task();
    USB_USBTask();

    if (timer_check_if_expired_reset(&hid_lights_expiry_timer)) {
      set_hid_standby_lighting(&led_state_from_hid_report);
    }

    process_tt(tt_x);
    int8_t tt1_report = analog_turntable_poll(&tt1, tt_x.tt_position);

    process_buttons(tt1_report);

    process_combos(&current_config,
                   &combo_timer,
                   &combo_lights_timer);

    bpm.update(button_state);

    update_lighting(tt1_report,
                    &combo_lights_timer,
                    bpm);
  }
}

// this refers to the hardware timer peripheral
// unrelated to the timer class in timer.h
void hardware_timer1_init() {
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
void hwinit() {
  // disable watchdog if enabled by bootloader/fuses
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // disable clock division
  clock_prescale_set(clock_div_1);

  // hardware init
  USB_Init();
  hardware_timer1_init();

  set_hid_standby_lighting(&led_state_from_hid_report);
}

// event handler for USB config change event
void EVENT_USB_Device_ConfigurationChanged() {
  // setup HID report endpoints
  Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
}

// process last received report from the host.
void ProcessGenericHIDReport(hid_lights led_state) {
  reactive_led = false;
  rgb_standby = false;
  timer_arm(&hid_lights_expiry_timer, HID_LIGHTS_EXPIRY_TIME);

  // update the lighting data
  led_state_from_hid_report = led_state;
}

void HID_Task() {
  Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

  // check if a packet has been sent from the host
  if (Endpoint_IsOUTReceived()) {
    // check if packet contains data
    if (Endpoint_IsReadWriteAllowed()) {
      //temp buffer to hold the read in report from the host
      hid_lights led_state;

      // read generic report data
      Endpoint_Read_Stream_LE(&led_state, sizeof(led_state), nullptr);

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
  auto JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

  // Infinitas only reads bits 1-7, 9-12,
  // so bit-shift bits 8 and up once
  uint8_t upper = button_state >> 7;
  uint8_t lower = button_state & 0x7F;
  JoystickReport->X = tt_x.tt_position / current_config.tt_ratio;
  JoystickReport->Button = (upper << 8) | lower;

  *ReportSize = sizeof(USB_JoystickReport_Data_t);
  return false;
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

void set_hid_standby_lighting(hid_lights* lights) {
  reactive_led = true;
  rgb_standby = true;

  lights->buttons = 0;
}

void process_buttons(int8_t tt1_report) {
  static DebounceState effectors_debounce(4);

  button_state = 0;
  for (int i = 0; i < BUTTONS; ++i) {
    process_button(buttons[i].INPUT_PORT.PIN,
                   i,
                   buttons[i].input_pin);
  }

  switch (tt1_report) {
  case -1:
    button_state |= BUTTON_TT_NEG;
    break;
  case 1:
    button_state |= BUTTON_TT_POS;
    break;
  default:
    break;
  }

  debounce(&effectors_debounce, EFFECTORS_ALL);
}

void process_button(const volatile uint8_t* PIN,
                    uint8_t button_number,
                    uint8_t input_pin) {
  bool pressed = ~*PIN & (1 << input_pin);
  button_state |= pressed << button_number;
}

void update_tt_transitions(uint8_t reverse_tt) {
  const int8_t direction = reverse_tt ? -1 : 1;
  const int8_t tt_transitions_values[4][4] = {
      {0, direction, -direction, 0},
      {-direction, 0, 0, direction},
      {direction, 0, 0, -direction},
      {0, -direction, direction, 0}
  };
  memcpy(tt_transitions, tt_transitions_values, sizeof(tt_transitions));
}

void process_tt(tt_pins &tt_pin) {
  // tt logic
  // example where tt_x wired to F0/F1:
  // curr is binary number ab
  // where a is the signal of F0
  // and b is the signal of F1
  // therefore when F0 == 1 and F1 == 0, then curr == 0b10
  int8_t a = (*tt_pin.PIN >> tt_pin.a_pin) & 1;
  int8_t b = (*tt_pin.PIN >> tt_pin.b_pin) & 1;
  int8_t curr = (a << 1) | b;

  auto direction = tt_transitions[tt_pin.prev][curr];
  tt_pin.tt_position += direction;
  tt_pin.tt_position %= 256 * current_config.tt_ratio;
  tt_pin.prev = curr;
}

void update_lighting(int8_t tt1_report,
                     timer* combo_lights_timer,
                     const Bpm &bpm) {
  if (reactive_led) {
    update_button_lighting(button_state,
                           combo_lights_timer);
  } else {
    update_button_lighting(led_state_from_hid_report.buttons,
                           combo_lights_timer);
  }

  if (!current_config.disable_led) {
    // update lighting on a timer to reduce the number of
    // computationally expensive calls to FastLED.show()
    // basically what FastLED does but without spin waiting
    static uint32_t last_show = 0;
    uint32_t min_micros = 1000000 / BEEF_LED_REFRESH;
    uint32_t now = micros();
    if (now-last_show < min_micros)
      return;
    last_show = now;

    RgbManager::Turntable::update(tt1_report,
                                  led_state_from_hid_report.tt_lights,
                                  current_config);
    RgbManager::Bar::update(led_state_from_hid_report.bar_lights,
                            current_config,
                            bpm);

    FastLED.show();
  }
}

void update_button_lighting(uint16_t led_state,
                            timer* combo_lights_timer) {
  if (current_config.disable_led) {
    led_state = 0;
  }

  if (timer_is_active(combo_lights_timer)) {
    // Temporarily black out button LEDs to notify a mode change
    led_state = 0;
  }

  for (int i = 0; i < BUTTONS; ++i) {
    set_led(buttons[i].LED_PORT.PORT,
            i,
            buttons[i].led_pin,
            led_state);
  }
}

bool is_pressed(uint16_t button_bits, uint16_t ignore) {
  return (button_state & ~ignore) == button_bits;
}

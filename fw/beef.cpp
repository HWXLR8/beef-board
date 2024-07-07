#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "devices/iidx/iidx_usb.h"
#include "devices/sdvx/sdvx_usb.h"
#include "Descriptors.h"

#include "analog_button.h"
#include "beef.h"
#include "combo.h"
#include "debounce.h"
#include "rgb_helper.h"

USB_ClassInfo_HID_Device_t* hid_interface;

void (*update_callback) (const config &);
bool (*create_hid_report_callback) (USB_ClassInfo_HID_Device_t* const,
                                    uint8_t* const,
                                    const uint8_t,
                                    void*,
                                    uint16_t* const);

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state = 0;

// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
bool reactive_led = true;
bool rgb_standby = true;
int8_t tt_transitions[4][4];
button_pins buttons[] = CONFIG_ALL_HW_PIN;

timer hid_lights_expiry_timer;
timer combo_lights_timer;

tt_pins tt_x = { &PINF, PINF0, PINF1, -1, 0 };
tt_pins tt_y = { &PINF, PINF2, PINF3, -1, 0 };

encoder_pin encoder_x = { PINF4 };
encoder_pin encoder_y = { PINF5 };

config current_config;

ISR(TIMER1_COMPA_vect) {
  milliseconds++;
}

void debounce(DebounceState* debounce, uint16_t mask) {
  button_state =
    (button_state & ~mask) |
    (debounce->debounce(button_state & mask));
}

void check_for_dfu() {
  if (is_only_pressed(BUTTON_1 | BUTTON_2)) {
    jump_to_bootloader();
  }
}

int main() {
  hwinit();

  config_init(&current_config);
  usb_init(current_config);

  timer_init(&hid_lights_expiry_timer);
  timer_init(&combo_lights_timer);

  analog_button_init(&button_x, current_config.tt_deadzone, 200, true);
  analog_button_init(&button_y, current_config.tt_deadzone, 200, true);

  RgbHelper::init();

  while (true) {
    HID_Device_USBTask(hid_interface);
    USB_USBTask();

    if (USB_DeviceState != DEVICE_STATE_Configured)
      continue;

    if (timer_check_if_expired_reset(&hid_lights_expiry_timer)) {
      set_hid_standby_lighting();
    }

    process_buttons();
    process_combos(&current_config, &combo_lights_timer);
    update_callback(current_config);
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

void adc_init() {
  // Set reference voltage to AVcc and left-adjust result
  ADMUX = (1 << REFS0) | (1 << ADLAR);
  // Enable ADC and set prescaler to 128 for 125KHz sample rate (16MHz / 128 = 125KHz)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// configure board hardware and chip peripherals
void hwinit() {
  // disable watchdog if enabled by bootloader/fuses
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // disable clock division
  clock_prescale_set(clock_div_1);

  // hardware init
  hardware_timer1_init();
  adc_init();

  set_hid_standby_lighting();

  // tt_x DATA lines wired to F0/F1
  // tt_y DATA lines wired to F2/F3
  // encoder_x and encoder_y will be automatically set up by the ADC
  DDRF  &= 0b11110000;
  PORTF |= 0b00001111;

  for (auto &button : buttons) {
    CONFIG_DDR_INPUT(button.INPUT_PORT.DDR, button.input_pin);
    CONFIG_DDR_LED(button.LED_PORT.DDR, button.led_pin);

    CONFIG_PORT_INPUT(button.INPUT_PORT.PORT, button.input_pin);
    CONFIG_PORT_LED(button.LED_PORT.PORT, button.led_pin);
  }

  GlobalInterruptEnable();

  // Check for boot up combos
  process_buttons();

  // check if we need to jump to bootloader
  check_for_dfu();
}

void usb_init(config &config) {
  switch (button_state) {
    case BUTTON_1 | BUTTON_8:
      set_mode(config, UsbMode::IIDX);
      set_input_mode(config, InputMode::Joystick);
      break;
    case BUTTON_2 | BUTTON_8:
      set_mode(config, UsbMode::IIDX);
      set_input_mode(config, InputMode::Keyboard);
      break;
    case BUTTON_1 | BUTTON_9:
      set_mode(config, UsbMode::SDVX);
      set_input_mode(config, InputMode::Joystick);
      break;
    case BUTTON_2 | BUTTON_9:
      set_mode(config, UsbMode::SDVX);
      set_input_mode(config, InputMode::Keyboard);
      break;
    default:
      break;
  }

  switch (config.usb_mode) {
    case UsbMode::IIDX:
      IIDX::usb_init(config);
      break;
    case UsbMode::SDVX:
      SDVX::usb_init(config);
      break;
  }
  set_hid_interface(config);

  USB_Init();
}

void set_hid_interface(config &config) {
  switch (config.usb_mode) {
    case UsbMode::IIDX:
      IIDX::update_hid_interface(config);
      break;
    case UsbMode::SDVX:
      SDVX::update_hid_interface(config);
      break;
  }
}

// event handler for USB config change event
void EVENT_USB_Device_ConfigurationChanged() {
  // setup HID report endpoints
  Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);

  // Setup keyboard report endpoint
  Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize) {
  return create_hid_report_callback(HIDInterfaceInfo, ReportID, ReportType, ReportData, ReportSize);
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

void set_hid_standby_lighting() {
  reactive_led = true;
  rgb_standby = true;
}

void process_buttons() {
  button_state = 0;
  for (int i = 0; i < BUTTONS; ++i) {
    process_button(buttons[i].INPUT_PORT.PIN,
                   i,
                   buttons[i].input_pin);
  }
}

void process_button(const volatile uint8_t* PIN,
                    const uint8_t button_number,
                    const uint8_t input_pin) {
  const bool pressed = ~*PIN & (1 << input_pin);
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

void process_tt(tt_pins &tt_pin, uint8_t tt_ratio) {
  // tt logic
  // example where tt_x wired to F0/F1:
  // curr is binary number ab
  // where a is the signal of F0
  // and b is the signal of F1
  // therefore when F0 == 1 and F1 == 0, then curr == 0b10
  const uint8_t a = (*tt_pin.PIN >> tt_pin.a_pin) & 1;
  const uint8_t b = (*tt_pin.PIN >> tt_pin.b_pin) & 1;
  const uint8_t curr = (a << 1) | b;

  const auto direction = tt_transitions[tt_pin.prev][curr];
  tt_pin.tt_position += direction;
  tt_pin.tt_position %= 256 * tt_ratio;
  tt_pin.prev = curr;
}

void process_encoder(encoder_pin &encoder_pin) {
  // Select ADC channel
  ADMUX = (ADMUX & 0xF8) | (encoder_pin.pin & 0x07);
  // Start conversion
  ADCSRA |= (1 << ADSC);
  // Wait for conversion to finish
  while (ADCSRA & (1 << ADSC));
  // Read 8-bits of ADC value
  encoder_pin.position = ADCH;
}

void process_keyboard(uint8_t* const hid_key_codes,
                      const uint8_t* const key_codes,
                      const uint8_t n) {
  uint8_t used_key_codes = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (is_pressed(1 << i)) {
      hid_key_codes[used_key_codes++] = key_codes[i];
    }
  }
}

void update_lighting(uint16_t hid_buttons) {
  if (reactive_led) {
    update_button_lighting(button_state);
  } else {
    update_button_lighting(hid_buttons);
  }
}

void update_button_lighting(uint16_t led_state) {
  if (current_config.disable_led ||
    // Temporarily black out button LEDs to notify a setting change
    timer_is_active(&combo_lights_timer)) {
    led_state = 0;
  }

  for (int i = 0; i < BUTTONS; ++i) {
    set_led(buttons[i].LED_PORT.PORT,
            i,
            buttons[i].led_pin,
            led_state);
  }
}

bool is_only_pressed(uint16_t button_bits, uint16_t ignore) {
  return (button_state & ~ignore) == button_bits;
}

bool is_pressed(uint16_t button_bits, uint16_t ignore) {
  return (button_state & ~ignore) & button_bits;
}

void jump_to_bootloader() {
  // disable interrupts
  cli();
  // clear registers
  UDCON = 1;
  USBCON = (1<<FRZCLK);
  UCSR1B = 0;
  _delay_ms(5);
  EIMSK = 0;
  PCICR = 0;
  SPCR = 0;
  ACSR = 0;
  EECR = 0;
  ADCSRA = 0;
  TIMSK0 = 0;
  TIMSK1 = 0;
  TIMSK2 = 0;
  TIMSK3 = 0;
  UCSR1B = 0;
  TWCR = 0;
  DDRA = 0;
  DDRB = 0;
  DDRC = 0;
  DDRD = 0;
  DDRE = 0;
  DDRF = 0;
  PORTA = 0;
  PORTB = 0;
  PORTC = 0;
  PORTD = 0;
  PORTE = 0;
  PORTF = 0;
  asm volatile("jmp 0xF000");
}

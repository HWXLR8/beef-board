#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "devices/iidx/iidx_usb.h"
#include "devices/sdvx/sdvx_usb.h"
#include "Descriptors.h"

#include "axis.h"
#include "beef.h"
#include "combo.h"
#include "debounce.h"
#include "pin.h"
#include "rgb_helper.h"

// bit-field storing button state. bits 0-10 map to buttons 1-11
// bits 11 and 12 map to digital tt -/+
uint16_t button_state;
AbstractUsbHandler* usb_handler;
button_pins buttons[] = CONFIG_ALL_HW_PIN;

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
  SetupHardware();

  config_init(&current_config);
  usb_init(current_config);

  timer hid_lights_expiry_timer;
  timer_arm(&hid_lights_expiry_timer, 0);
  timer combo_lights_timer;
  timer_init(&combo_lights_timer);

  // Ignore button inputs after startup so that we don't send keycodes after holding a boot combo
  bool ignore_buttons = button_state;

  RgbHelper::init();

  while (true) {
    usb_handler->usb_task(current_config);

    set_hid_standby_lighting(hid_lights_expiry_timer);
    ignore_buttons = process_buttons(ignore_buttons);
    process_combos(&current_config, &combo_lights_timer);
    usb_handler->update(current_config,
                        hid_lights_expiry_timer,
                        combo_lights_timer);
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
void SetupHardware() {
  // disable watchdog if enabled by bootloader/fuses
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // disable clock division
  clock_prescale_set(clock_div_1);

  // hardware init
  hardware_timer1_init();
  adc_init();

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
  process_buttons(false);

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

  USB_Init();
}

// event handler for USB config change event
void EVENT_USB_Device_ConfigurationChanged() {
  // setup HID report endpoints
  Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(MOUSE_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);

  // We don't use StartOfFrame events to poll as it's too slow and results in lots of jitter from inputs
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize) {
  return usb_handler->create_hid_report(HIDInterfaceInfo,
                                        ReportID,
                                        ReportType,
                                        ReportData,
                                        ReportSize);
}

void set_led(volatile uint8_t* PORT,
             const uint8_t button_number,
             const uint8_t led_pin,
             const uint16_t led_state) {
  const uint8_t mask = 1 << led_pin;
  const uint8_t state = (led_state >> button_number) & 1;

  *PORT = (*PORT & ~mask) | // Clear led_pin bit
    (-state & mask);        // state will either be 0 or 255 (underflow)
}

void set_hid_standby_lighting(timer &hid_lights_expiry_timer) {
  const auto hid_expiry = timer_is_expired(&hid_lights_expiry_timer);

  reactive_led = hid_expiry;
  rgb_standby = hid_expiry;
}

bool process_buttons(bool ignore_buttons) {
  button_state = 0;
  for (uint8_t i = 0; i < BUTTONS; i++) {
    process_button(buttons[i].INPUT_PORT.PIN,
                   i,
                   buttons[i].input_pin);
  }

  // Ignore button inputs after startup
  ignore_buttons = ignore_buttons && button_state;
  // If we are still ignoring button inputs, clear button_state
  // Otherwise, retain
  button_state *= !ignore_buttons;

  return ignore_buttons;
}

void process_button(const volatile uint8_t* PIN,
                    const uint8_t button_number,
                    const uint8_t input_pin) {
  const bool pressed = ~*PIN & (1 << input_pin);
  button_state |= pressed << button_number;
}

void update_tt_transitions(uint8_t reverse_tt) {
  const int8_t direction = reverse_tt ? -1 : 1;
  const int8_t opposite_direction = -direction;
  const int8_t tt_transitions_values[4][4] = {
      {0, direction, opposite_direction, 0},
      {opposite_direction, 0, 0, direction},
      {direction, 0, 0, opposite_direction},
      {0, opposite_direction, direction, 0}
  };
  memcpy(tt_transitions, tt_transitions_values, sizeof(tt_transitions));
}

void process_keyboard(Beef::USB_KeyboardReport_Data_t* const hid_key_codes,
                      const uint8_t* const key_codes,
                      const uint8_t n) {
  uint8_t used_key_codes = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (is_pressed(1 << i)) {
      hid_key_codes->KeyCode[used_key_codes++] = key_codes[i];
    }
  }
}

void update_button_lighting(const config &config,
                            timer &combo_lights_timer,
                            uint16_t led_state) {
  if (reactive_led) {
    led_state = button_state;
  }

  if (config.disable_led ||
      // Temporarily black out button LEDs to notify a setting change
      timer_is_active(&combo_lights_timer)) {
    led_state = 0;
  }

  for (uint8_t i = 0; i < BUTTONS; i++) {
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
  GlobalInterruptDisable();
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

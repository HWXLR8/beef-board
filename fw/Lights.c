#include <avr/io.h>
#include <avr/interrupt.h>
#include "Config.h"
#include "Lights.h"

#define L07_DDR  DDRD
#define L07_PORT PORTD
#define L07_PIN  PIND

#define L8F_DDR  DDRB
#define L8F_PORT PORTB
#define L8F_PIN  PINB

Settings_Lights_t *SettingsLights;
Settings_Device_t *SettingsDevice;

uint16_t LightsData;

// Function for initializing lighting.
void Lights_Init(void) {
	// We'll turn off interrupts temporarily during setup procedures.
	cli();

  // We're not really using any settings here in this implementation.
  // However, we can expose configuration objects if we need them.
  Config_AddressLights(&SettingsLights);
	Config_AddressDevice(&SettingsDevice);

	// The reference implementation for lights supports 12 lights, and uses a multiplexing setup for shared I/O with buttons.
	//
	// This implementation works off of the following parts:
	// * Two CY74FCT573T latches
	// * One 10k resistor per I/O pin
	//
	// Wire the following:
	// * Each button should be wired to an I/O pin through a 10k in-line resistor. BTN-------10k---I/O
	// * Each I/O pin should be wired directly to an input on the CY74FCT573T.     CY74:IN---------I/O
	// * Wire /OE on the CY74FCT573T to ground.                                    CY74:/OE--------GND
	// * Wire  LE on the CY74FCT573T to PF7.                                       CY74:LE---------PF7
  // * Wire  LE to ground through a 22k pull-down resistor.                      CY74:LE---10k---GND
  // * Wire each output on the CY74FCT573T to a light. Avoid directly driving LEDs; consider using a MOSFET or transistor and the necessary support components.
  //                                                                             CY74:OUT--------FET
	// * Wire each unused input on the CY74FCT573T to ground.                      CY74:IN---------GND
  //
  // This specific implementation uses the following pins for I/O:
  // * PD0-PD7
  // * PB4-PB7 (PB0-PB3 are the SPI bus used for PS2)

  // For setup, we only configure the latching pin.
  //DDRF |= 0x80;

	// Since setup is done, we can re-enable interrupts.
	sei();
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

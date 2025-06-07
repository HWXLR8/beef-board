import type { HsvaColor } from "svelte-awesome-color-picker";

export enum TurntableMode {
  Static = 'Static',
  Spin = 'Spin',
  Shift = 'Shift',
  RainbowStatic = 'Rainbow Static',
  RainbowReactive = 'Rainbow Reactive',
  RainbowSpin = 'Rainbow Spin',
  Reactive = 'Reactive',
  Breathing = 'Breathing',
  HID = 'HID',
  Off = 'Off'
}

export enum BarMode {
  KeySpectrumP1 = 'Key Spectrum (P1)',
  KeySpectrumP2 = 'Key Spectrum (P2)',
  HID = 'HID',
  TapeLedP1 = 'Tape LED (P1)',
  TapeLedP2 = 'Tape LED (P2)',
  Off = 'Off'
}

export enum ControllerType {
  IIDX = 'IIDX',
  SDVX = 'SDVX'
}

export enum InputMode {
  Joystick = 'Joystick',
  Keyboard = 'Keyboard'
}

// Mapping Layers
export const turntableModeToNumber: { [key in TurntableMode]: number } = {
  [TurntableMode.Static]: 0,
  [TurntableMode.Spin]: 1,
  [TurntableMode.Shift]: 2,
  [TurntableMode.RainbowStatic]: 3,
  [TurntableMode.RainbowReactive]: 4,
  [TurntableMode.RainbowSpin]: 5,
  [TurntableMode.Reactive]: 6,
  [TurntableMode.Breathing]: 7,
  [TurntableMode.HID]: 8,
  [TurntableMode.Off]: 9
};

export const numberToTurntableMode: { [key: number]: TurntableMode } = Object.fromEntries(
  Object.entries(turntableModeToNumber).map(([key, value]) => [value, key as TurntableMode])
);

export const barModeToNumber: { [key in BarMode]: number } = {
  [BarMode.KeySpectrumP1]: 1,
  [BarMode.KeySpectrumP2]: 2,
  [BarMode.HID]: 4,
  [BarMode.TapeLedP1]: 5,
  [BarMode.TapeLedP2]: 6,
  [BarMode.Off]: 7
};

export const numberToBarMode: { [key: number]: BarMode } = Object.fromEntries(
  Object.entries(barModeToNumber).map(([key, value]) => [value, key as BarMode])
);

export const controllerTypeToNumber: { [key in ControllerType]: number } = {
  [ControllerType.IIDX]: 0,
  [ControllerType.SDVX]: 1
};

export const numberToControllerType: { [key: number]: ControllerType } = Object.fromEntries(
  Object.entries(controllerTypeToNumber).map(([key, value]) => [value, key as ControllerType])
);

export const inputModeToNumber: { [key in InputMode]: number } = {
  [InputMode.Joystick]: 0,
  [InputMode.Keyboard]: 1
};

export const numberToInputMode: { [key: number]: InputMode } = Object.fromEntries(
  Object.entries(inputModeToNumber).map(([key, value]) => [value, key as InputMode])
);

export class Hsv {
  h = $state(0);
  s = $state(0);
  v = $state(0);

  constructor(h: number, s: number, v: number) {
    this.h = (h * 360) / 255;
    this.s = (s * 100) / 255;
    this.v = (v * 100) / 255;
  }

  fromHsva(color: HsvaColor): void {
    this.h = color.h;
    this.s = color.s;
    this.v = color.v;
  }

  toHid(): [number, number, number] {
    return [(this.h * 255) / 360, (this.s * 255) / 100, (this.v * 255) / 100];
  }
}

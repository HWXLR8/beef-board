import type { HsvaColor } from 'colord';
import type { Config } from '$lib/types/config';

export enum TurntableMode {
  Static = 0,
  Spin = 1,
  Shift = 2,
  RainbowStatic = 3,
  RainbowReactive = 4,
  RainbowSpin = 5,
  Reactive = 6,
  Breathing = 7,
  HID = 8,
  Off = 9
}

export enum BarMode {
  KeySpectrumP1 = 1,
  KeySpectrumP2 = 2,
  HID = 4,
  Off = 5
}

export enum ControllerType {
  IIDX = 0,
  SDVX = 1
}

export enum InputMode {
  Joystick = 0,
  Keyboard = 1
}

export class Hsv {
  h: number;
  s: number;
  v: number;

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

export class HsvEnumMapping {
  displayName: string;
  hsvField?: keyof Config;

  constructor(displayName: string, hsvField?: keyof Config) {
    this.displayName = displayName;
    this.hsvField = hsvField;
  }
}

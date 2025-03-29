import { get } from 'svelte/store';
import { ReportId } from '$lib/types/hid';
import { device } from '$lib/types/state';
import { TurntableMode, BarMode, ControllerType, InputMode, Hsv } from '$lib/types/types';
import * as HIDCodes from '$lib/types/hid-codes';

export class IIDXKeyMapping {
  public main_buttons: number[];
  public function_buttons: number[];
  public tt_ccw: number;
  public tt_cw: number;
  public padding: number[];

  constructor() {
    this.main_buttons = [
      HIDCodes.HID_KEYBOARD_SC_S, // 1
      HIDCodes.HID_KEYBOARD_SC_D, // 2
      HIDCodes.HID_KEYBOARD_SC_F, // 3
      HIDCodes.HID_KEYBOARD_SC_SPACE, // 4
      HIDCodes.HID_KEYBOARD_SC_J, // 5
      HIDCodes.HID_KEYBOARD_SC_K, // 6
      HIDCodes.HID_KEYBOARD_SC_L // 7
    ];
    this.function_buttons = [
      HIDCodes.HID_KEYBOARD_SC_1_AND_EXCLAMATION, // E1/Start
      HIDCodes.HID_KEYBOARD_SC_2_AND_AT, // E2
      HIDCodes.HID_KEYBOARD_SC_3_AND_HASHMARK, // E3
      HIDCodes.HID_KEYBOARD_SC_4_AND_DOLLAR // E4/Select
    ];
    this.tt_ccw = HIDCodes.HID_KEYBOARD_SC_DOWN_ARROW; // TT-
    this.tt_cw = HIDCodes.HID_KEYBOARD_SC_UP_ARROW; // TT+
    this.padding = Array(7).fill(0);
  }
}

export class SDVXKeyMapping {
  public bt_buttons: number[];
  public fx_buttons: number[];
  public padding_1: number[];
  public start: number;
  public padding_2: number[];

  constructor() {
    this.bt_buttons = [
      HIDCodes.HID_KEYBOARD_SC_D, // BT-A
      HIDCodes.HID_KEYBOARD_SC_F, // BT-B
      HIDCodes.HID_KEYBOARD_SC_J, // BT-C
      HIDCodes.HID_KEYBOARD_SC_K // BT-D
    ];
    this.fx_buttons = [
      HIDCodes.HID_KEYBOARD_SC_C, // FX-L
      HIDCodes.HID_KEYBOARD_SC_M // FX-R
    ];
    this.padding_1 = Array(2).fill(0);
    this.start = HIDCodes.HID_KEYBOARD_SC_ENTER; // Start
    this.padding_2 = Array(11).fill(0);
  }
}

export class Config {
  version: number;
  reverse_tt: boolean;
  tt_effect: TurntableMode;
  tt_deadzone: number;
  bar_effect: BarMode;
  disable_leds: boolean;
  tt_static_hsv: Hsv;
  tt_spin_hsv: Hsv;
  tt_shift_hsv: Hsv;
  tt_rainbow_static_hsv: Hsv;
  tt_rainbow_react_hsv: Hsv;
  tt_rainbow_spin_hsv: Hsv;
  tt_react_hsv: Hsv;
  tt_breathing_hsv: Hsv;
  tt_ratio: number;
  controller_type: ControllerType;
  iidx_input_mode: InputMode;
  sdvx_input_mode: InputMode;
  tt_sustain_ms: number = 0;
  iidx_keys: IIDXKeyMapping = new IIDXKeyMapping();
  sdvx_keys: SDVXKeyMapping = new SDVXKeyMapping();

  constructor(configData: DataView) {
    this.version = configData.getUint8(0);
    this.reverse_tt = configData.getUint8(1) as unknown as boolean;
    this.tt_effect = configData.getUint8(2) as TurntableMode;
    this.tt_deadzone = configData.getUint8(3);
    this.bar_effect = configData.getUint8(4) as BarMode;
    this.disable_leds = configData.getUint8(5) as unknown as boolean;
    this.tt_static_hsv = new Hsv(
      configData.getUint8(6),
      configData.getUint8(7),
      configData.getUint8(8)
    );
    this.tt_spin_hsv = new Hsv(
      configData.getUint8(9),
      configData.getUint8(10),
      configData.getUint8(11)
    );
    this.tt_shift_hsv = new Hsv(
      configData.getUint8(12),
      configData.getUint8(13),
      configData.getUint8(14)
    );
    this.tt_rainbow_static_hsv = new Hsv(
      configData.getUint8(15),
      configData.getUint8(16),
      configData.getUint8(17)
    );
    this.tt_rainbow_react_hsv = new Hsv(
      configData.getUint8(18),
      configData.getUint8(19),
      configData.getUint8(20)
    );
    this.tt_rainbow_spin_hsv = new Hsv(
      configData.getUint8(21),
      configData.getUint8(22),
      configData.getUint8(23)
    );
    this.tt_react_hsv = new Hsv(
      configData.getUint8(24),
      configData.getUint8(25),
      configData.getUint8(26)
    );
    this.tt_breathing_hsv = new Hsv(
      configData.getUint8(27),
      configData.getUint8(28),
      configData.getUint8(29)
    );
    this.tt_ratio = configData.getUint8(30);
    this.controller_type = configData.getUint8(31) as ControllerType;
    this.iidx_input_mode = configData.getUint8(32) as InputMode;
    this.sdvx_input_mode = configData.getUint8(33) as InputMode;

    if (this.version >= 12) {
      this.tt_sustain_ms = configData.getUint8(34);
    }

    // Read key mappings if version supports it
    if (this.version >= 13) {
      let offset = 35;

      // Read IIDX key mappings)
      for (let i = 0; i < this.iidx_keys.main_buttons.length; i++) {
        this.iidx_keys.main_buttons[i] = configData.getUint8(offset++);
      }

      for (let i = 0; i < this.iidx_keys.function_buttons.length; i++) {
        this.iidx_keys.function_buttons[i] = configData.getUint8(offset++);
      }

      this.iidx_keys.tt_ccw = configData.getUint8(offset++);
      this.iidx_keys.tt_cw = configData.getUint8(offset++);

      // Skip padding
      offset += this.iidx_keys.padding.length;

      // Read SDVX key mappings
      for (let i = 0; i < this.sdvx_keys.bt_buttons.length; i++) {
        this.sdvx_keys.bt_buttons[i] = configData.getUint8(offset++);
      }

      for (let i = 0; i < this.sdvx_keys.fx_buttons.length; i++) {
        this.sdvx_keys.fx_buttons[i] = configData.getUint8(offset++);
      }

      offset += this.sdvx_keys.padding_1.length;

      this.sdvx_keys.start = configData.getUint8(offset++);

      // Skip padding
      offset += this.sdvx_keys.padding_2.length;
    }
  }
}

const packetSize = 1024;

export async function readConfig(): Promise<Config> {
  const dev = get(device);
  if (!dev) {
    throw new Error('Device not connected');
  }

  try {
    const result = await dev.receiveFeatureReport(ReportId.Config);
    const configData = new DataView(result.buffer.slice(1)); // Skip report id

    const version = configData.getUint8(0);
    if (version <= 10) {
      throw new Error('Firmware version is too old. Please flash the latest firmware build');
    }

    return new Config(configData);
  } catch (err) {
    return Promise.reject(new Error('Failed to read config', { cause: err }));
  }
}

export async function updateConfig(config: Config): Promise<void> {
  const dev = get(device);
  if (!dev) {
    throw new Error('Device not connected');
  }

  try {
    const configBuffer = new ArrayBuffer(packetSize);
    const configView = new DataView(configBuffer);

    let i = 0;
    for (const [_, value] of Object.entries(config)) {
      if (value instanceof Hsv) {
        const [h, s, v] = value.toHid();
        configView.setUint8(i++, h);
        configView.setUint8(i++, s);
        configView.setUint8(i++, v);
        continue;
      }

      if (value instanceof IIDXKeyMapping) {
        for (const v of value.main_buttons) {
          configView.setUint8(i++, Number(v));
        }
        for (const v of value.function_buttons) {
          configView.setUint8(i++, Number(v));
        }
        configView.setUint8(i++, value.tt_ccw);
        configView.setUint8(i++, value.tt_cw);
        i += value.padding.length;
        continue;
      }

      if (value instanceof SDVXKeyMapping) {
        for (const v of value.bt_buttons) {
          configView.setUint8(i++, Number(v));
        }
        for (const v of value.fx_buttons) {
          configView.setUint8(i++, Number(v));
        }
        i += value.padding_1.length;
        configView.setUint8(i++, value.start);
        i += value.padding_2.length;
        continue;
      }

      configView.setUint8(i++, Number(value));
    }

    const data = new Uint8Array(configBuffer);
    await dev.sendFeatureReport(ReportId.Config, data);
  } catch (err) {
    return Promise.reject(new Error('Failed to update config', { cause: err }));
  }
}

import { ReportId } from '$lib/types/hid';
import { appState } from '$lib/types/state.svelte';
import { TurntableMode, BarMode, ControllerType, InputMode, Hsv, numberToTurntableMode, numberToBarMode, numberToControllerType, numberToInputMode, turntableModeToNumber, barModeToNumber, controllerTypeToNumber, inputModeToNumber } from '$lib/types/types.svelte';
import * as HIDCodes from '$lib/types/hid-codes';

const packetSize = 1024;

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
  version = $state(0);
  reverse_tt = $state(false);
  tt_effect = $state(TurntableMode.Static);
  tt_deadzone = $state(0);
  bar_effect = $state(BarMode.KeySpectrumP1);
  disable_leds = $state(false);
  tt_static_hsv = $state(new Hsv(0, 0, 0));
  tt_spin_hsv = $state(new Hsv(0, 0, 0));
  tt_shift_hsv = $state(new Hsv(0, 0, 0));
  tt_rainbow_static_hsv = $state(new Hsv(0, 0, 0));
  tt_rainbow_react_hsv = $state(new Hsv(0, 0, 0));
  tt_rainbow_spin_hsv = $state(new Hsv(0, 0, 0));
  tt_react_hsv = $state(new Hsv(0, 0, 0));
  tt_breathing_hsv = $state(new Hsv(0, 0, 0));
  tt_ratio = $state(0);
  controller_type = $state(ControllerType.IIDX);
  iidx_input_mode = $state(InputMode.Joystick);
  sdvx_input_mode = $state(InputMode.Joystick);
  tt_sustain_ms = $state(0);
  iidx_keys = $state(new IIDXKeyMapping());
  sdvx_keys = $state(new SDVXKeyMapping());
  iidx_buttons_debounce = $state(0);
  iidx_effectors_debounce = $state(0);
  sdvx_buttons_debounce = $state(0);
  led_refresh = $state(0);
  rainbow_spin_speed = $state(0);
  tt_leds = $state(0);

  constructor(configData: DataView) {
    this.version = configData.getUint8(0);
    this.reverse_tt = configData.getUint8(1) as unknown as boolean;
    this.tt_effect = numberToTurntableMode[configData.getUint8(2)];
    this.tt_deadzone = configData.getUint8(3);
    this.bar_effect = numberToBarMode[configData.getUint8(4)];
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
    this.controller_type = numberToControllerType[configData.getUint8(31)];
    this.iidx_input_mode = numberToInputMode[configData.getUint8(32)];
    this.sdvx_input_mode = numberToInputMode[configData.getUint8(33)];

    if (this.version >= 12) {
      this.tt_sustain_ms = configData.getUint8(34);
    }

    // Read key mappings if version supports it
    let offset = 35;
    if (this.version >= 13) {
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

    if (this.version >= 15) {
      this.iidx_buttons_debounce = configData.getUint8(offset++);
      this.iidx_effectors_debounce = configData.getUint8(offset++);
      this.sdvx_buttons_debounce = configData.getUint8(offset++);
    }

    if (this.version >= 16) {
      this.led_refresh = configData.getUint8(offset++);
      this.rainbow_spin_speed = configData.getUint8(offset++);
      this.tt_leds = configData.getUint8(offset++);
    }
  }
}

export async function readConfig(): Promise<Config> {
  if (!appState.device) {
    throw new Error('Device not connected');
  }

  try {
    const result = await appState.device.receiveFeatureReport(ReportId.Config);
    const configData = new DataView(result.buffer.slice(1)); // Skip report id

    const version = configData.getUint8(0);
    if (version <= 10) {
      throw new Error('Firmware version is too old. Please flash the latest firmware build');
    }

    return new Config(configData);
  } catch (err) {
    throw new Error(`Failed to read config: ${err}`);
  }
}

export async function updateConfig(config: Config): Promise<void> {
  if (!appState.device) {
    throw new Error('Device not connected');
  }

  console.log("updating config")

  try {
    const configBuffer = new ArrayBuffer(packetSize);
    const configView = new DataView(configBuffer);

    configView.setUint8(0, config.version);
    configView.setUint8(1, Number(config.reverse_tt));
    configView.setUint8(2, turntableModeToNumber[config.tt_effect]);
    configView.setUint8(3, config.tt_deadzone);
    configView.setUint8(4, barModeToNumber[config.bar_effect]);
    configView.setUint8(5, Number(config.disable_leds));
    let [h, s, v] = config.tt_static_hsv.toHid();
    configView.setUint8(6, h);
    configView.setUint8(7, s);
    configView.setUint8(8, v);
    [h, s, v] = config.tt_spin_hsv.toHid();
    configView.setUint8(9, h);
    configView.setUint8(10, s);
    configView.setUint8(11, v);
    [h, s, v] = config.tt_shift_hsv.toHid();
    configView.setUint8(12, h);
    configView.setUint8(13, s);
    configView.setUint8(14, v);
    [h, s, v] = config.tt_rainbow_static_hsv.toHid();
    configView.setUint8(15, h);
    configView.setUint8(16, s);
    configView.setUint8(17, v);
    [h, s, v] = config.tt_rainbow_react_hsv.toHid();
    configView.setUint8(18, h);
    configView.setUint8(19, s);
    configView.setUint8(20, v);
    [h, s, v] = config.tt_rainbow_spin_hsv.toHid();
    configView.setUint8(21, h);
    configView.setUint8(22, s);
    configView.setUint8(23, v);
    [h, s, v] = config.tt_react_hsv.toHid();
    configView.setUint8(24, h);
    configView.setUint8(25, s);
    configView.setUint8(26, v);
    [h, s, v] = config.tt_breathing_hsv.toHid();
    configView.setUint8(27, h);
    configView.setUint8(28, s);
    configView.setUint8(29, v);
    configView.setUint8(30, config.tt_ratio);
    configView.setUint8(31, controllerTypeToNumber[config.controller_type]);
    configView.setUint8(32, inputModeToNumber[config.iidx_input_mode]);
    configView.setUint8(33, inputModeToNumber[config.sdvx_input_mode]);

    // Write tt_sustain_ms if version supports it
    if (config.version >= 12) {
      configView.setUint8(34, config.tt_sustain_ms);
    }

    // Write key mappings if version supports it
    let offset = 35;
    if (config.version >= 13) {
      // Write IIDX key mappings
      for (let i = 0; i < config.iidx_keys.main_buttons.length; i++) {
        configView.setUint8(offset++, config.iidx_keys.main_buttons[i]);
      }

      for (let i = 0; i < config.iidx_keys.function_buttons.length; i++) {
        configView.setUint8(offset++, config.iidx_keys.function_buttons[i]);
      }

      configView.setUint8(offset++, config.iidx_keys.tt_ccw);
      configView.setUint8(offset++, config.iidx_keys.tt_cw);

      // Skip padding
      offset += config.iidx_keys.padding.length;

      // Write SDVX key mappings
      for (let i = 0; i < config.sdvx_keys.bt_buttons.length; i++) {
        configView.setUint8(offset++, config.sdvx_keys.bt_buttons[i]);
      }

      for (let i = 0; i < config.sdvx_keys.fx_buttons.length; i++) {
        configView.setUint8(offset++, config.sdvx_keys.fx_buttons[i]);
      }

      // Skip padding
      offset += config.sdvx_keys.padding_1.length;

      configView.setUint8(offset++, config.sdvx_keys.start);

      // Skip padding
      offset += config.sdvx_keys.padding_2.length;
    }

    if (config.version >= 15) {
      configView.setUint8(offset++, config.iidx_buttons_debounce);
      configView.setUint8(offset++, config.iidx_effectors_debounce);
      configView.setUint8(offset++, config.sdvx_buttons_debounce);
    }

    if (config.version >= 16) {
      configView.setUint8(offset++, config.led_refresh);
      configView.setUint8(offset++, config.rainbow_spin_speed);
      configView.setUint8(offset++, config.tt_leds);
    }

    const data = new Uint8Array(configBuffer);
    await appState.device.sendFeatureReport(ReportId.Config, data);
  } catch (err) {
    throw new Error(`Failed to update config: ${err}`);
  }
}
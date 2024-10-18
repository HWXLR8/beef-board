import { ReportId } from '$lib/types/hid';
import { TurntableMode, BarMode, ControllerType, InputMode, Hsv } from '$lib/types/types';

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
  }
}

const packetSize = 64;

export async function readConfig(dev: HIDDevice): Promise<Config> {
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

export async function updateConfig(dev: HIDDevice, config: Config): Promise<void> {
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

      configView.setUint8(i++, Number(value));
    }

    const data = new Uint8Array(configBuffer);
    await dev.sendFeatureReport(ReportId.Config, data);
  } catch (err) {
    return Promise.reject(new Error('Failed to update config', { cause: err }));
  }
}

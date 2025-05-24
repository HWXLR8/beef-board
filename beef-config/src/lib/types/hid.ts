import { appState, onDisconnect, connectDevice } from '$lib/types/state.svelte';

export enum ReportId {
  Config = 1,
  Command = 2,
  FirmwareVersion = 3
}

export enum Command {
  Bootloader = 1,
  ResetConfig = 2
}

export async function waitForReconnection(): Promise<void> {
  await onDisconnect().then(connectDevice);
}

export async function detectDevice(): Promise<HIDDevice | null> {
  const devices = await navigator.hid.requestDevice({
    filters: [
      { vendorId: 0x1ccf, productId: 0x8048, usagePage: 0xffeb, usage: 0x01 }, // IIDX
      { vendorId: 0x1ccf, productId: 0x101c, usagePage: 0xffeb, usage: 0x01 }  // SDVX
    ]
  });

  if (devices.length === 0) {
    return null;
  }

  // Shouldn't be necessary, but let's play it safe
  const selectedDevice = devices.find((d) => d.productName === 'BEEF BOARD');
  if (!selectedDevice) {
    throw new Error('Invalid device found, is this an official Konami controller?');
  }

  if (!selectedDevice.opened) {
    await selectedDevice.open();
  }
  return selectedDevice;
}

export async function readCommitHash(): Promise<string> {
  if (!appState.device) {
    throw new Error('Device not connected');
  }

  try {
    const result = await appState.device.receiveFeatureReport(ReportId.FirmwareVersion);
    const hashData = new DataView(result.buffer.slice(1)); // Skip report id
    return hashData.getUint32(0, true).toString(16).padStart(8, '0');
  } catch (err) {
    throw new Error('Failed to read commit hash', { cause: err });
  }
}

export async function sendCommand(command: Command): Promise<void> {
  if (!appState.device) {
    throw new Error('Device not connected');
  }

  try {
    const commandData = new Uint8Array([command]);
    await appState.device.sendFeatureReport(ReportId.Command, commandData);
  } catch (err) {
    throw new Error('Failed to send command', { cause: err });
  }
}

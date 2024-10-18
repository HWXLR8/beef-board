export enum ReportId {
  Config = 1,
  Command = 2,
  FirmwareVersion = 3
}

export enum Command {
  Bootloader = 1,
  ResetConfig = 2
}

export async function detectDevice(): Promise<HIDDevice> {
  try {
    const devices = await navigator.hid.requestDevice({
      filters: [
        { vendorId: 0x1ccf, productId: 0x8048, usagePage: 0xffeb, usage: 0x01 },
        { vendorId: 0x1ccf, productId: 0x1014, usagePage: 0xffeb, usage: 0x01 }
      ]
    });

    if (devices.length === 0) {
      throw new Error('No controller found');
    }

    // Shouldn't be necessary, but let's play it safe
    const selectedDevice = devices.find((d) => d.productName === 'BEEF BOARD');
    if (!selectedDevice) {
      throw new Error('Invalid device found, is this an official Konami controller?');
    }

    if (!selectedDevice.opened) {
      await selectedDevice.open();
    }
    return Promise.resolve(selectedDevice);
  } catch (err) {
    return Promise.reject(err);
  }
}

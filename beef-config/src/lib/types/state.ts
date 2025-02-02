import { detectDevice } from "$lib/types/hid";
import { get, writable } from "svelte/store";

export const disableConfigTab = writable(false);
export const device = writable<HIDDevice | undefined>(undefined);
export const error = writable<string | undefined>(undefined);

export async function onDisconnect(): Promise<void> {
  await get(device)?.close();
  disableConfigTab.set(false);
  device.set(undefined);
  error.set(undefined);
}

export async function connectDevice(): Promise<void> {
  try {
    const dev = await detectDevice();
    if (dev === null) {
      return;
    }
    device.set(dev);
    navigator.hid.addEventListener('disconnect', onDisconnect);
    error.set(undefined);
  } catch (err) {
    await onDisconnect();
    error.set(`Error communicating with device: ${err}`);
    throw err;
  }
}

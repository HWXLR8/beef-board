import { detectDevice } from "$lib/types/hid";

class AppState {
  disableConfigTab = $state(false);
  device = $state<HIDDevice | null>(null);
  error = $state<string | undefined>();
}

export const appState = new AppState();

export async function onDisconnect(): Promise<void> {
  await appState.device?.close();
  appState.disableConfigTab = false;
  appState.device = null;
  appState.error = undefined;
}

export async function connectDevice(): Promise<void> {
  try {
    appState.device = await detectDevice();
    if (appState.device === null) {
      return;
    }
    navigator.hid.addEventListener('disconnect', onDisconnect);
    appState.error = undefined;
  } catch (err) {
    await onDisconnect();
    appState.error = `Error communicating with device: ${err}`;
    throw err;
  }
}

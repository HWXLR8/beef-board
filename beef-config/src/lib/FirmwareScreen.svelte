<script lang="ts">
	import { onMount } from 'svelte';
	import { get } from 'svelte/store';
	import MemoryMap from 'nrf-intel-hex';
	import { persisted } from 'svelte-persisted-store';

	import { Alert, AlertDescription, AlertTitle } from '$lib/components/ui/alert';
	import * as AlertDialog from '$lib/components/ui/alert-dialog';
	import { Button } from '$lib/components/ui/button';
	import { Checkbox } from '$lib/components/ui/checkbox';
	import { Label } from '$lib/components/ui/label';
	import { Input } from '$lib/components/ui/input';
	import { Progress } from '$lib/components/ui/progress';

	import WarningAlert from '$lib/WarningAlert.svelte';

	import { ATMEL_MAX_TRANSFER_SIZE, DfuDevice } from '$lib/types/dfu';
	import { Command, readCommitHash, sendCommand, waitForReconnection } from '$lib/types/hid';
	import { onDisconnect, appState } from '$lib/types/state.svelte';

	const Stage = {
		Standby: 'standby',
		Flashing: 'flashing',
		Finished: 'finished'
	};

	let commitHash = $state('');
	let dfuDevice: DfuDevice | undefined = $state();
	let firmwareData = $state(new MemoryMap());
	let flashing = $state(false);
	let message = $state('Ready');
	let progress = $state(0);
	let stage = $state(Stage.Standby);

	const hideFirmwareWarningStorage = persisted('hideFirmwareWarning', false);

	onMount(async () => {
		try {
			commitHash = await readCommitHash();
		} catch (err) {
			appState.error = `Error reading commit hash: ${err}`;
		}
	});

	async function rebootDevice(): Promise<void> {
		navigator.hid.removeEventListener('disconnect', onDisconnect);
		appState.disableConfigTab = true;
		await sendCommand(Command.Bootloader);
		stage = Stage.Flashing;
		await connectToDfuDevice();
	}

	async function connectToDfuDevice(): Promise<void> {
		try {
			const dev = await DfuDevice.connect();
			if (dev === null) {
				return;
			}
			dfuDevice = dev;
			dfuDevice.on('message', (value) => {
				message = value;
			});
			dfuDevice.on('progress', (value) => {
				progress = value;
			});

			navigator.usb.addEventListener('disconnect', onDisconnect);
		} catch (err) {
			appState.error = `Error connecting to bootloader: ${err}`;
			throw err;
		}
	}

	function parseFirmwareFile(event: Event): void {
		const target = event.target as HTMLInputElement;
		const file = target.files?.[0];

		if (file) {
			const reader = new FileReader();

			// Read the file as text
			reader.readAsText(file);

			reader.onload = () => {
				const data = reader.result as string;
				firmwareData = MemoryMap.fromHex(data, ATMEL_MAX_TRANSFER_SIZE);
			};

			// Handle errors
			reader.onerror = () => {
				appState.error = `Error reading firmware file: ${reader.error}`;
			};
		}
	}

	async function flashFirmware(): Promise<void> {
		flashing = true;
		onbeforeunload = () => {
			return 'Flashing is in progress. Are you sure you want to leave?';
		};

		try {
			await dfuDevice!.downloadFirmware(firmwareData);
		} catch (err) {
			appState.error = `Error flashing firmware: ${err}`;
			throw err;
		} finally {
			onbeforeunload = null;
		}

		stage = Stage.Finished;
	}

	async function finishFlashing(): Promise<void> {
		navigator.usb.removeEventListener('disconnect', onDisconnect);
		if (dfuDevice) {
			await dfuDevice.startApplication();
			await dfuDevice.device.close();
			await waitForReconnection();
		}
	}
</script>

{#if stage == Stage.Standby}
	<h3 class="mb-2 text-lg font-semibold">Firmware Commit: <code>0x{commitHash}</code></h3>

	<Button onclick={rebootDevice} class="mb-4">Start Firmware Flash</Button>
{:else if stage == Stage.Flashing}
	{#if !dfuDevice}
		<Button onclick={connectToDfuDevice} class="mb-4">Connect to Bootloader</Button>
		<Button onclick={onDisconnect} class="mb-4" variant="destructive">Cancel</Button>

		<Alert class="mb-4">
			<AlertTitle>Having trouble connecting to the bootloader?</AlertTitle>
			<AlertDescription class="inline">
				You probably need to install/reinstall the required drivers. <a
					href="https://github.com/qmk/qmk_driver_installer/releases/latest"
					style="text-decoration: underline">Download and run qmk_driver_installer.exe</a
				>, and retry the flashing process.
			</AlertDescription>
		</Alert>
	{:else}
		<h3 class="mb-2 text-lg font-semibold">Connected to Bootloader</h3>

		<div class="mb-2 grid w-full max-w-sm items-center gap-1.5">
			<Label for="fw">Select Firmware to Flash</Label>
			<Input name="fw" type="file" accept=".hex" onchange={parseFirmwareFile} />
		</div>

		{#if firmwareData}
			<h3 class="mb-2 text-lg font-semibold">{message}</h3>
			<Progress value={progress} max={1} class="mb-4 w-full" />

			{#if flashing}
				<WarningAlert
					title="Now flashing"
					description="Do not disconnect the device or close the browser tab until flashing is complete."
					variant="destructive"
				></WarningAlert>
			{:else}
				{#if !get(hideFirmwareWarningStorage)}
					<AlertDialog.Root>
						<AlertDialog.Trigger>
							{#snippet child({ props })}
								<Button {...props}>Flash Firmware</Button>
							{/snippet}
						</AlertDialog.Trigger>
						<AlertDialog.Content>
							<AlertDialog.Header>
								<AlertDialog.Title>Are you sure?</AlertDialog.Title>
								<AlertDialog.Description>
									⚠️ WARNING: Only flash firmware made for Beef Board. Flashing unknown firmware
									could damage your device. Click Continue to proceed with flashing, or Cancel to
									abort.
								</AlertDialog.Description>
							</AlertDialog.Header>
							<div class="items-top flex space-x-2">
								<Checkbox id="warning" bind:checked={$hideFirmwareWarningStorage} />
								<div class="grid gap-1.5 leading-none">
									<Label for="warning" class="text-sm">Don't remind me again</Label>
								</div>
							</div>
							<AlertDialog.Footer>
								<AlertDialog.Action onclick={flashFirmware}>Continue</AlertDialog.Action>
								<AlertDialog.Cancel>Cancel</AlertDialog.Cancel>
							</AlertDialog.Footer>
						</AlertDialog.Content>
					</AlertDialog.Root>
				{:else}
					<Button onclick={flashFirmware} class="mb-4">Flash Firmware</Button>
				{/if}
				<Button onclick={finishFlashing} class="mb-4" variant="destructive">Cancel</Button>
			{/if}
		{:else}
			<Button onclick={finishFlashing} class="mb-4" variant="destructive">Cancel</Button>
		{/if}
	{/if}
{:else if stage == Stage.Finished}
	<h3 class="mb-2 text-lg font-semibold">Firmware Flashed Successfully</h3>
	<Progress value={1} max={1} class="mb-4 w-full" />
	<Button onclick={finishFlashing} class="mb-4">Exit</Button>
{/if}

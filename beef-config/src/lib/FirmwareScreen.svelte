<script lang="ts">
	import { onMount } from 'svelte';
	import MemoryMap from 'nrf-intel-hex';

	import { Button } from '$lib/components/ui/button';
	import { Label } from '$lib/components/ui/label';
	import { Input } from '$lib/components/ui/input';
	import Progress from '$lib/components/ui/progress/progress.svelte';

	import WarningAlert from '$lib/WarningAlert.svelte';

	import { ATMEL_MAX_TRANSFER_SIZE, DfuDevice } from '$lib/types/dfu';
	import { Command, readCommitHash, sendCommand, waitForReconnection } from '$lib/types/hid';
	import { onDisconnect, error, disableConfigTab } from '$lib/types/state';

	const Stage = {
		Standby: 'standby',
		Flashing: 'flashing',
		Finished: 'finished'
	};

	let commitHash: string;
	let dfuDevice: DfuDevice | undefined;
	let firmwareData: MemoryMap;
	let flashing = false;
	let message = 'Ready';
	let progress = 0;
	let stage = Stage.Standby;

	onMount(async () => {
		try {
			commitHash = await readCommitHash();
		} catch (err) {
			error.set(`Error reading commit hash: ${err}`);
		}
	});

	async function rebootDevice(): Promise<void> {
		navigator.hid.removeEventListener('disconnect', onDisconnect);
		disableConfigTab.set(true);
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
			error.set(`Error connecting to bootloader: ${err}`);
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
				error.set('Error reading firmware file');
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
			error.set(`Error flashing firmware: ${err}`);
			throw err;
		} finally {
			onbeforeunload = null;
		}

		stage = Stage.Finished;
	}

	async function finishFlashing(): Promise<void> {
		navigator.usb.removeEventListener('disconnect', onDisconnect);
		await dfuDevice!.startApplication();
		await dfuDevice!.device.close();
		await waitForReconnection();
	}
</script>

{#if stage == Stage.Standby}
	<h3 class="mb-2 text-lg font-semibold">Firmware Commit: <code>0x{commitHash}</code></h3>

	<Button on:click={rebootDevice} class="mb-4">Start Firmware Flash</Button>
{:else if stage == Stage.Flashing}
	{#if !dfuDevice}
		<Button on:click={connectToDfuDevice} class="mb-4">Connect to Bootloader</Button>
		<Button on:click={onDisconnect} class="mb-4" variant="destructive">Cancel</Button>
	{:else}
		<h3 class="mb-2 text-lg font-semibold">Connected to Bootloader</h3>

		<div class="mb-2 grid w-full max-w-sm items-center gap-1.5">
			<Label for="fw">Select Firmware to Flash</Label>
			<Input name="fw" type="file" accept=".hex" on:change={parseFirmwareFile} />
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
				<Button on:click={flashFirmware} class="mb-4">Flash Firmware</Button>
				<Button on:click={finishFlashing} class="mb-4" variant="destructive">Cancel</Button>
			{/if}
		{:else}
			<Button on:click={finishFlashing} class="mb-4" variant="destructive">Cancel</Button>
		{/if}
	{/if}
{:else if stage == Stage.Finished}
	<h3 class="mb-2 text-lg font-semibold">Firmware Flashed Successfully</h3>
	<Progress value={1} max={1} class="mb-4 w-full" />
	<Button on:click={finishFlashing} class="mb-4">Exit</Button>
{/if}

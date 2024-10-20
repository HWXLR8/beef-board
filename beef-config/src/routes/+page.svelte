<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import { TriangleAlert } from 'lucide-svelte';

	import { Alert } from '$lib/components/ui/alert';
	import AlertDescription from '$lib/components/ui/alert/alert-description.svelte';
	import * as AlertDialog from '$lib/components/ui/alert-dialog/index.js';
	import AlertTitle from '$lib/components/ui/alert/alert-title.svelte';
	import { Button } from '$lib/components/ui/button';
	import { Label } from '$lib/components/ui/label/index.js';
	import * as Select from '$lib/components/ui/select';
	import { Separator } from '$lib/components/ui/separator/index.js';
	import { Slider } from '$lib/components/ui/slider';
	import { Skeleton } from '$lib/components/ui/skeleton/index.js';

	import InputModes from '$lib/InputModes.svelte';
	import LightEffectSelect from '$lib/LightEffectSelect.svelte';
	import Switch from '$lib/Switch.svelte';
	import { Config, readConfig, updateConfig } from '$lib/types/config';
	import { ReportId, Command, detectDevice } from '$lib/types/hid';
	import { TurntableMode, BarMode, ControllerType, EnumMapping } from '$lib/types/types';

	let device: HIDDevice | null = null;
	let config: Config | null = null;
	let commitHash: string | null = null;
	let error: string | null = null;
	let isWebHIDSupported = false;
	let loading = true;
	let controllerTypeChanged = false;

	const ttModeMapping: Record<TurntableMode, EnumMapping> = {
		[TurntableMode.Static]: new EnumMapping('Static', 'tt_static_hsv'),
		[TurntableMode.Spin]: new EnumMapping('Spin', 'tt_spin_hsv'),
		[TurntableMode.Shift]: new EnumMapping('Shift', 'tt_shift_hsv'),
		[TurntableMode.RainbowStatic]: new EnumMapping('Rainbow Static', 'tt_rainbow_static_hsv'),
		[TurntableMode.RainbowReactive]: new EnumMapping('Rainbow Reactive', 'tt_rainbow_react_hsv'),
		[TurntableMode.RainbowSpin]: new EnumMapping('Rainbow Spin', 'tt_rainbow_spin_hsv'),
		[TurntableMode.Reactive]: new EnumMapping('Reactive', 'tt_react_hsv'),
		[TurntableMode.Breathing]: new EnumMapping('Breathing', 'tt_breathing_hsv'),
		[TurntableMode.HID]: new EnumMapping('HID'),
		[TurntableMode.Off]: new EnumMapping('Off')
	} as const;

	const barModeMapping: Record<BarMode, EnumMapping> = {
		[BarMode.KeySpectrumP1]: new EnumMapping('Key Spectrum (P1)'),
		[BarMode.KeySpectrumP2]: new EnumMapping('Key Spectrum (P2)'),
		[BarMode.HID]: new EnumMapping('HID'),
		[BarMode.Off]: new EnumMapping('Off')
	} as const;

	export const controllerTypes = Object.values(ControllerType)
		.filter((v) => isNaN(Number(v)))
		.map((_, index) => ({
			value: index,
			label: ControllerType[index]
		}));

	onMount(() => {
		checkWebHIDSupport();
		loading = false;
	});

	onDestroy(onDisconnect);

	// This gets fired twice when first loading config data,
	// apparently will get fixed in Svelte 5?
	$: if (device && config) {
		try {
			updateConfig(device, config);
		} catch (err) {
			error = `Failed to update config: ${err}`;
		}
	}

	function checkWebHIDSupport(): void {
		if ('hid' in navigator) {
			isWebHIDSupported = true;
		} else {
			error =
				'WebHID is not supported in this browser. Please use a compatible browser like Chrome or Edge.';
		}
	}

	async function onDisconnect(): Promise<void> {
		await device?.close();
		device = null;
		config = null;
		commitHash = null;
		error = null;
	}

	async function connectDevice(): Promise<void> {
		try {
			device = await detectDevice();
			await readCommitHash(device);
			config = await readConfig(device);
			navigator.hid.addEventListener('disconnect', onDisconnect);
		} catch (err) {
			error = `Error communicating with device: ${err}`;
			await onDisconnect();
			return Promise.reject(err);
		}
	}

	async function readCommitHash(dev: HIDDevice): Promise<void> {
		try {
			const result = await dev.receiveFeatureReport(ReportId.FirmwareVersion);
			const hashData = new DataView(result.buffer.slice(1)); // Skip report id
			commitHash = hashData.getUint32(0, true).toString(16).padStart(8, '0');
		} catch (err) {
			err = new Error('Failed to read commit hash', { cause: err });
			return Promise.reject(err);
		}
	}

	async function waitForReconnection(): Promise<void> {
		await onDisconnect().then(connectDevice);
	}

	async function sendCommand(device: HIDDevice, command: Command): Promise<void> {
		try {
			const commandData = new Uint8Array([command]);
			await device.sendFeatureReport(ReportId.Command, commandData).catch((err) => {
				return Promise.reject(new Error('Failed to send command', { cause: err }));
			});
		} catch (err) {
			error = `Failed to send command: ${err}`;
			return Promise.reject(err);
		}
	}
</script>

<main class="mx-auto max-w-screen-lg p-4">
	<h1 class="mb-4 text-2xl font-bold">Beef Board Configuration</h1>

	{#if loading}
		<Skeleton class="h-10" />
	{:else if !isWebHIDSupported}
		<Alert variant="destructive">
			<AlertTitle>Browser Not Supported</AlertTitle>
			<AlertDescription>
				WebHID is not supported in this browser. Please use a compatible browser like Chrome or
				Edge.
			</AlertDescription>
		</Alert>
	{:else}
		{#if !device}
			<Button on:click={connectDevice} class="mb-4">Connect Device</Button>
		{:else if config}
			<div class="mb-4">
				<Label for="controller-type">Controller Mode</Label>
				<Select.Root
					selected={{
						value: config.controller_type,
						label: ControllerType[config.controller_type]
					}}
					onSelectedChange={(value) => {
						if (config && value) {
							config.controller_type = value.value;
							controllerTypeChanged = true;
						}
					}}
				>
					<Select.Trigger class="w-[180px]">
						<Select.Value placeholder="Controller mode" />
					</Select.Trigger>
					<Select.Content>
						<Select.Group>
							{#each controllerTypes as type}
								<Select.Item value={type.value} label={type.label}>{type.label}</Select.Item>
							{/each}
						</Select.Group>
					</Select.Content>
				</Select.Root>
			</div>

			{#if controllerTypeChanged}
				<Alert class="mb-4">
					<TriangleAlert class="h-4 w-4" />
					<AlertTitle>Heads up!</AlertTitle>
					<AlertDescription
						>Controller mode changed. Please replug the controller to apply the new mode.</AlertDescription
					>
				</Alert>
			{/if}

			<h3 class="mb-2 text-lg font-semibold">Firmware Commit: <code>0x{commitHash}</code></h3>

			<Separator class="mb-4" />

			{#if config.controller_type === ControllerType.IIDX}
				<div>
					<h3 class="mb-2 text-lg font-semibold">IIDX Configuration</h3>
					<InputModes bind:inputMode={config.iidx_input_mode} />
					<Switch label="Reverse Turntable" bind:checked={config.reverse_tt} />
					<div class="mb-4">
						<Label for="tt-deadzone">Turntable Deadzone</Label>
						<Slider
							id="tt-deadzone"
							min={1}
							max={6}
							step={1}
							value={[config.tt_deadzone]}
							onValueChange={(value) => {
								if (config) {
									config.tt_deadzone = value[0];
								}
							}}
						/>
					</div>
					<div class="mb-4">
						<Label for="tt-ratio">Turntable Sensitivity</Label>
						<!-- We store TT ratio but present it as TT sensitivity, so invert the range -->
						<Slider
							id="tt-ratio"
							min={1}
							max={6}
							step={1}
							value={[7 - config.tt_ratio]}
							onValueChange={(value) => {
								if (config) {
									config.tt_ratio = 7 - value[0];
								}
							}}
						/>
					</div>
					<Switch label="Disable LEDs" bind:checked={config.disable_leds} />
					{#if !config.disable_leds}
						<LightEffectSelect
							label="Turntable Effect"
							bind:config
							bind:effect={config.tt_effect}
							modeMapping={ttModeMapping}
						/>

						<LightEffectSelect
							label="Light Bar Effect"
							bind:config
							bind:effect={config.bar_effect}
							modeMapping={barModeMapping}
						/>
					{/if}
				</div>
			{:else if config.controller_type === ControllerType.SDVX}
				<div>
					<h3 class="mb-2 text-lg font-semibold">SDVX Configuration</h3>
					<InputModes bind:inputMode={config.sdvx_input_mode} />
					<Switch label="Disable LEDs" bind:checked={config.disable_leds} />
				</div>
			{/if}

			<Separator class="mb-4" />

			<div class="mt-4">
				<Button
					on:click={async () => {
						if (device) {
							await sendCommand(device, Command.Bootloader);
						}
					}}
					class="mr-2">Enter Bootloader</Button
				>
				<AlertDialog.Root closeOnOutsideClick={true}>
					<AlertDialog.Trigger asChild let:builder>
						<Button builders={[builder]} variant="destructive">Reset Config</Button>
					</AlertDialog.Trigger>
					<AlertDialog.Content>
						<AlertDialog.Header>
							<AlertDialog.Title>Are you sure?</AlertDialog.Title>
							<AlertDialog.Description>
								This action cannot be undone. This will reset all settings to their default values
								and the controller will disconnect.
							</AlertDialog.Description>
						</AlertDialog.Header>
						<AlertDialog.Footer>
							<AlertDialog.Cancel>Cancel</AlertDialog.Cancel>
							<AlertDialog.Action
								on:click={async () => {
									if (device) {
										await sendCommand(device, Command.ResetConfig).then(waitForReconnection);
									}
								}}>Continue</AlertDialog.Action
							>
						</AlertDialog.Footer>
					</AlertDialog.Content>
				</AlertDialog.Root>
			</div>
		{/if}

		{#if error}
			<Alert variant="destructive" class="mb-4">
				<AlertTitle>Error</AlertTitle>
				<AlertDescription>{error}</AlertDescription>
			</Alert>
		{/if}
	{/if}
</main>

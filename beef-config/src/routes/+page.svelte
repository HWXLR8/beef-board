<script lang="ts">
	import { onDestroy, onMount } from 'svelte';

	import ColorPicker from 'svelte-awesome-color-picker';
	import type { HsvaColor } from 'colord';
	import type { Selected } from 'bits-ui';
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
	import { Switch } from '$lib/components/ui/switch';

	// Enums
	const packetSize = 64;

	enum ReportId {
		Config = 1,
		Command = 2,
		FirmwareVersion = 3
	}

	enum Command {
		Bootloader = 1,
		ResetConfig = 2
	}

	enum TurntableMode {
		Static = 0,
		Spin = 1,
		Shift = 2,
		RainbowStatic = 3,
		RainbowReactive = 4,
		RainbowSpin = 5,
		Reactive = 6,
		Breathing = 7,
		HID = 8,
		Off = 9
	}

	enum BarMode {
		KeySpectrumP1 = 1,
		KeySpectrumP2 = 2,
		HID = 4,
		Off = 5
	}

	enum ControllerType {
		IIDX = 0,
		SDVX = 1
	}

	enum InputMode {
		Joystick = 0,
		Keyboard = 1
	}

	class Hsv {
		h: number;
		s: number;
		v: number;

		constructor(h: number, s: number, v: number) {
			this.h = (h * 360) / 255;
			this.s = (s * 100) / 255;
			this.v = (v * 100) / 255;
		}

		static fromHsv(color: HsvaColor): Hsv {
			const hsv = new this(0, 0, 0);
			hsv.h = color.h;
			hsv.s = color.s;
			hsv.v = color.v;
			return hsv;
		}

		toHid(): [number, number, number] {
			return [(this.h * 255) / 360, (this.s * 255) / 100, (this.v * 255) / 100];
		}
	}

	interface Config {
		version: number;
		reverse_tt: boolean;
		tt_effect: TurntableMode;
		tt_deadzone: number;
		bar_effect: BarMode;
		disable_led: boolean;
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
	}

	function configFromDataView(configData: DataView): Config {
		return {
			version: configData.getUint8(0),
			reverse_tt: configData.getUint8(1) as unknown as boolean,
			tt_effect: configData.getUint8(2) as TurntableMode,
			tt_deadzone: configData.getUint8(3),
			bar_effect: configData.getUint8(4) as BarMode,
			disable_led: configData.getUint8(5) as unknown as boolean,
			tt_static_hsv: new Hsv(
				configData.getUint8(6),
				configData.getUint8(7),
				configData.getUint8(8)
			),
			tt_spin_hsv: new Hsv(
				configData.getUint8(9),
				configData.getUint8(10),
				configData.getUint8(11)
			),
			tt_shift_hsv: new Hsv(
				configData.getUint8(12),
				configData.getUint8(13),
				configData.getUint8(14)
			),
			tt_rainbow_static_hsv: new Hsv(
				configData.getUint8(15),
				configData.getUint8(16),
				configData.getUint8(17)
			),
			tt_rainbow_react_hsv: new Hsv(
				configData.getUint8(18),
				configData.getUint8(19),
				configData.getUint8(20)
			),
			tt_rainbow_spin_hsv: new Hsv(
				configData.getUint8(21),
				configData.getUint8(22),
				configData.getUint8(23)
			),
			tt_react_hsv: new Hsv(
				configData.getUint8(24),
				configData.getUint8(25),
				configData.getUint8(26)
			),
			tt_breathing_hsv: new Hsv(
				configData.getUint8(27),
				configData.getUint8(28),
				configData.getUint8(29)
			),
			tt_ratio: configData.getUint8(30),
			controller_type: configData.getUint8(31) as ControllerType,
			iidx_input_mode: configData.getUint8(32) as InputMode,
			sdvx_input_mode: configData.getUint8(33) as InputMode
		};
	}

	let device: HIDDevice | null = null;
	let config: Config | null = null;
	let commitHash: string | null = null;
	let error: string | null = null;
	let isWebHIDSupported = false;
	let loading = true;

	let hsva = { h: 0, s: 0, v: 0 } as HsvaColor;
	let colorPickerElement: HTMLDivElement;
	let controllerTypeChanged = false;

	class EnumMapping {
		displayName: string;
		hsvField?: keyof Config;

		constructor(displayName: string, hsvField?: keyof Config) {
			this.displayName = displayName;
			this.hsvField = hsvField;
		}
	}
	const ttModeMapping = new Map<TurntableMode, EnumMapping>([
		[TurntableMode.Static, new EnumMapping('Static', 'tt_static_hsv')],
		[TurntableMode.Spin, new EnumMapping('Spin', 'tt_spin_hsv')],
		[TurntableMode.Shift, new EnumMapping('Shift', 'tt_shift_hsv')],
		[TurntableMode.RainbowStatic, new EnumMapping('Rainbow Static', 'tt_rainbow_static_hsv')],
		[TurntableMode.RainbowReactive, new EnumMapping('Rainbow Reactive', 'tt_rainbow_react_hsv')],
		[TurntableMode.RainbowSpin, new EnumMapping('Rainbow Spin', 'tt_rainbow_spin_hsv')],
		[TurntableMode.Reactive, new EnumMapping('Reactive', 'tt_react_hsv')],
		[TurntableMode.Breathing, new EnumMapping('Breathing', 'tt_breathing_hsv')],
		[TurntableMode.HID, new EnumMapping('HID')],
		[TurntableMode.Off, new EnumMapping('Off')]
	]);

	const barLightModes = new Map<BarMode, EnumMapping>([
		[BarMode.KeySpectrumP1, new EnumMapping('Key Spectrum (P1)')],
		[BarMode.KeySpectrumP2, new EnumMapping('Key Spectrum (P2)')],
		[BarMode.HID, new EnumMapping('HID')],
		[BarMode.Off, new EnumMapping('Off')]
	]);

	const controllerTypes = Object.values(ControllerType)
		.filter((v) => isNaN(Number(v)))
		.map((_, index) => ({
			value: index,
			label: ControllerType[index]
		}));

	const inputModes = Object.values(InputMode)
		.filter((v) => isNaN(Number(v)))
		.map((_, index) => ({
			value: index,
			label: InputMode[index]
		}));

	onMount(() => {
		checkWebHIDSupport();
		loading = false;
	});

	onDestroy(() => {
		onDisconnect();
	});

	function checkWebHIDSupport(): void {
		if ('hid' in navigator) {
			isWebHIDSupported = true;
		} else {
			error =
				'WebHID is not supported in this browser. Please use a compatible browser like Chrome or Edge.';
		}
	}

	async function onDisconnect(): Promise<void> {
		if (device) {
			await device.close();
		}
		error = null;
		device = null;
		config = null;
		commitHash = null;
	}

	async function detectDevice(): Promise<void> {
		try {
			const devices = await navigator.hid.requestDevice({
				filters: [
					{ vendorId: 0x1ccf, productId: 0x8048, usagePage: 0xffeb, usage: 0x01 },
					{ vendorId: 0x1ccf, productId: 0x1014, usagePage: 0xffeb, usage: 0x01 }
				]
			});

			if (devices.length > 0) {
				// Shouldn't be necessary, but let's play it safe
				const selectedDevice = devices.find((d) => d.productName === 'BEEF BOARD');
				if (!selectedDevice) {
					throw new Error('Invalid device found, is this an official Konami controller?');
				}

				if (!selectedDevice.opened) {
					await selectedDevice.open();
				}
				device = selectedDevice;
			}
		} catch (err) {
			error = `Failed to detect device: ${err}`;
			onDisconnect();
			return Promise.reject(err);
		}

		try {
			if (device) {
				await readCommitHash(device);
				await readConfig(device);
				navigator.hid.addEventListener('disconnect', onDisconnect);
			}
		} catch (err) {
			error = `Error communicating with device: ${err}`;
			onDisconnect();
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

	async function readConfig(dev: HIDDevice): Promise<void> {
		try {
			const result = await dev.receiveFeatureReport(ReportId.Config);
			const configData = new DataView(result.buffer.slice(1)); // Skip report id

			const version = configData.getUint8(0);
			if (version <= 10) {
				throw new Error('Firmware version is too old. Please flash the latest firmware build');
			}

			config = configFromDataView(configData);
			updateTurntableHsvBinding(config.tt_effect);
		} catch (err) {
			err = new Error('Failed to read config', { cause: err });
			return Promise.reject(err);
		}
	}

	async function updateConfig(newConfig: Config): Promise<void> {
		if (!device) {
			error = 'No device connected';
			return;
		}

		try {
			const configBuffer = new ArrayBuffer(packetSize);
			const configView = new DataView(configBuffer);

			let i = 0;
			for (const [_, value] of Object.entries(newConfig)) {
				if (value instanceof Hsv) {
					const hsv: Hsv = value;
					const [h, s, v] = hsv.toHid();
					configView.setUint8(i++, h);
					configView.setUint8(i++, s);
					configView.setUint8(i++, v);
					continue;
				}

				configView.setUint8(i++, value);
			}

			const data = new Uint8Array(configBuffer);
			await device.sendFeatureReport(ReportId.Config, data);
			config = newConfig;
		} catch (err) {
			error = `Failed to update config: ${err}`;
			return Promise.reject(err);
		}
	}

	async function sendCommand(command: Command): Promise<void> {
		if (!device) {
			error = 'No device connected';
			return;
		}

		try {
			const commandData = new Uint8Array([command]);
			await device.sendFeatureReport(ReportId.Command, commandData);
			if (command === Command.ResetConfig) {
				await waitForReconnection();
			}
		} catch (err) {
			error = `Failed to send command: ${err}`;
			return Promise.reject(err);
		}
	}

	async function waitForReconnection(): Promise<void> {
		onDisconnect();
		await detectDevice();
	}

	function handleConfigChange(field: keyof Config, value: any): void {
		if (config) {
			const newConfig = { ...config, [field]: value };
			updateConfig(newConfig);
		}
	}

	function handleEnumChange(
		enumType: any,
		field: keyof Config,
		value: Selected<number> | undefined
	): void {
		if (config && value) {
			if (!Object.values(enumType).includes(value.value)) {
				error = `Invalid value: ${value.value}`;
				return;
			}

			const newConfig = {
				...config,
				[field]: value.value
			};
			updateConfig(newConfig);
		}
	}

	// Limit updates so that we don't immediately wear out EEPROM if the user spasms out on the colour picker
	function handleTtHsvStart(event: PointerEvent): void {
		colorPickerElement.setPointerCapture(event.pointerId);
	}

	function handleTtHsvEnd(event: PointerEvent): void {
		colorPickerElement.releasePointerCapture(event.pointerId);
		handleTurntableHsvChange();
	}

	function handleTurntableHsvChange(): void {
		if (config) {
			handleHsvChange(ttModeMapping.get(config.tt_effect)?.hsvField);
		}
	}

	function handleHsvChange(field: keyof Config | undefined): void {
		if (config && field) {
			const hsv = Hsv.fromHsv(hsva);
			const newConfig = { ...config, [field]: hsv };
			updateConfig(newConfig);
		}
	}

	function updateTurntableHsvBinding(ttEffect: TurntableMode): void {
		if (config) {
			const field = ttModeMapping.get(ttEffect)?.hsvField;
			if (field && field in config) {
				const hsv = config[field] as Hsv;
				hsva.h = hsv.h;
				hsva.s = hsv.s;
				hsva.v = hsv.v;
			}
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
			<Button on:click={detectDevice} class="mb-4">Connect Device</Button>
		{:else if config}
			<div class="mb-4">
				<Label for="controller-type">Controller Mode</Label>
				<Select.Root
					selected={{
						value: config.controller_type,
						label: ControllerType[config.controller_type]
					}}
					onSelectedChange={(value) => {
						handleEnumChange(ControllerType, 'controller_type', value);
						controllerTypeChanged = true;
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
					<div class="mb-4">
						<Label for="iidx-input">Input Mode</Label>
						<Select.Root
							selected={{
								value: config.iidx_input_mode,
								label: InputMode[config.iidx_input_mode]
							}}
							onSelectedChange={(value) => handleEnumChange(InputMode, 'iidx_input_mode', value)}
						>
							<Select.Trigger class="w-[180px]">
								<Select.Value placeholder="Input Mode" />
							</Select.Trigger>
							<Select.Content>
								<Select.Group>
									{#each inputModes as mode}
										<Select.Item value={mode.value} label={mode.label}>{mode.label}</Select.Item>
									{/each}
								</Select.Group>
							</Select.Content>
						</Select.Root>
					</div>
					<div class="mb-4">
						<Label for="reverse-tt">Reverse Turntable</Label>
						<Switch
							id="reverse-tt"
							checked={config.reverse_tt}
							onCheckedChange={(value) => handleConfigChange('reverse_tt', value)}
						/>
					</div>
					<div class="mb-4">
						<Label for="tt-deadzone">Turntable Deadzone</Label>
						<Slider
							id="tt-deadzone"
							min={1}
							max={6}
							step={1}
							value={[config.tt_deadzone]}
							onValueChange={(value) => handleConfigChange('tt_deadzone', value[0])}
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
							onValueChange={(value) => handleConfigChange('tt_ratio', 7 - value[0])}
						/>
					</div>
					<div class="mb-4">
						<Label for="disable-led">Disable LEDs</Label>
						<Switch
							id="disable-led"
							checked={config.disable_led}
							onCheckedChange={(value) => handleConfigChange('disable_led', value)}
						/>
					</div>
					{#if !config.disable_led}
						<div class="mb-4">
							<Label for="tt-hsv">Turntable Effect</Label>
							<Select.Root
								selected={{
									value: config.tt_effect,
									label: ttModeMapping.get(config.tt_effect)?.displayName
								}}
								onSelectedChange={(value) => {
									if (value) {
										handleEnumChange(TurntableMode, 'tt_effect', value);
										updateTurntableHsvBinding(value.value);
									}
								}}
							>
								<Select.Trigger class="w-[180px]">
									<Select.Value placeholder="Turntable Effect" />
								</Select.Trigger>
								<Select.Content>
									<Select.Group>
										{#each ttModeMapping as [value, label]}
											<Select.Item {value} label={label.displayName}
												>{label.displayName}</Select.Item
											>
										{/each}
									</Select.Group>
								</Select.Content>
							</Select.Root>
						</div>
						{#if ttModeMapping.get(config.tt_effect)?.hsvField}
							<div class="mb-4">
								<Label for="tt-hsv">Turntable Effect HSV</Label>
								<div
									bind:this={colorPickerElement}
									role="none"
									on:pointerdown={handleTtHsvStart}
									on:pointerup={handleTtHsvEnd}
									on:input={handleTurntableHsvChange}
								>
									<ColorPicker position="responsive" isAlpha={false} on:input bind:hsv={hsva} />
								</div>
							</div>
						{/if}
						<div class="mb-4">
							<Label for="bar-hsv">Light Bar Effect</Label>
							<Select.Root
								selected={{
									value: config.bar_effect,
									label: barLightModes.get(config.bar_effect)?.displayName
								}}
								onSelectedChange={(value) => {
									handleEnumChange(BarMode, 'bar_effect', value);
								}}
							>
								<Select.Trigger class="w-[180px]">
									<Select.Value placeholder="Light Bar Effect" />
								</Select.Trigger>
								<Select.Content>
									<Select.Group>
										{#each barLightModes as [value, label]}
											<Select.Item {value} label={label.displayName}
												>{label.displayName}</Select.Item
											>
										{/each}
									</Select.Group>
								</Select.Content>
							</Select.Root>
						</div>
					{/if}
				</div>
			{:else if config.controller_type === ControllerType.SDVX}
				<div>
					<h3 class="mb-2 text-lg font-semibold">SDVX Configuration</h3>
					<div class="mb-4">
						<Label for="sdvx-input">Input Mode</Label>
						<Select.Root
							selected={{
								value: config.sdvx_input_mode,
								label: InputMode[config.sdvx_input_mode]
							}}
							onSelectedChange={(value) => handleEnumChange(InputMode, 'sdvx_input_mode', value)}
						>
							<Select.Trigger class="w-[180px]">
								<Select.Value placeholder="Input Mode" />
							</Select.Trigger>
							<Select.Content>
								<Select.Group>
									{#each inputModes as mode}
										<Select.Item value={mode.value} label={mode.label}>{mode.label}</Select.Item>
									{/each}
								</Select.Group>
							</Select.Content>
						</Select.Root>
					</div>
					<div class="mb-4">
						<Label for="disable-led">Disable LEDs</Label>
						<Switch
							id="disable-led"
							checked={config.disable_led}
							onCheckedChange={(value) => handleConfigChange('disable_led', value)}
						/>
					</div>
				</div>
			{/if}

			<Separator class="mb-4" />

			<div class="mt-4">
				<Button on:click={() => sendCommand(Command.Bootloader)} class="mr-2"
					>Enter Bootloader</Button
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
							<AlertDialog.Action on:click={() => sendCommand(Command.ResetConfig)}
								>Continue</AlertDialog.Action
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

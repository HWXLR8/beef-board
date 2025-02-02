<script lang="ts">
	import { onMount } from 'svelte';

	import * as AlertDialog from '$lib/components/ui/alert-dialog/index.js';
	import { Button } from '$lib/components/ui/button';
	import { Label } from '$lib/components/ui/label/index.js';
	import * as Select from '$lib/components/ui/select';
	import { Separator } from '$lib/components/ui/separator/index.js';

	import InputModes from '$lib/InputModes.svelte';
	import LightEffectSelect from '$lib/LightEffectSelect.svelte';
	import SliderInput from '$lib/SliderInput.svelte';
	import Switch from '$lib/Switch.svelte';
	import ToolTipLabel from '$lib/ToolTipLabel.svelte';

	import { readConfig, updateConfig, type Config } from '$lib/types/config';
	import { Command, sendCommand, waitForReconnection } from '$lib/types/hid';
	import { error } from '$lib/types/state';
	import { TurntableMode, HsvEnumMapping, BarMode, ControllerType } from '$lib/types/types';
	import WarningAlert from './WarningAlert.svelte';

	let config: Config | undefined;
	let controllerTypeChanged = false;

	onMount(async () => {
		try {
			config = await readConfig();
		} catch (err) {
			error.set(`Failed to read config: ${err}`);
		}
	});

	// This gets fired twice when first loading config data,
	// apparently will get fixed in Svelte 5?
	$: {
		if (config) {
			try {
				updateConfig(config);
			} catch (err) {
				error.set(`Failed to update config: ${err}`);
			}
		}
	}

	const ttModeMapping: Record<TurntableMode, HsvEnumMapping> = {
		[TurntableMode.Static]: new HsvEnumMapping('Static', 'tt_static_hsv'),
		[TurntableMode.Spin]: new HsvEnumMapping('Spin', 'tt_spin_hsv'),
		[TurntableMode.Shift]: new HsvEnumMapping('Shift', 'tt_shift_hsv'),
		[TurntableMode.RainbowStatic]: new HsvEnumMapping('Rainbow Static', 'tt_rainbow_static_hsv'),
		[TurntableMode.RainbowReactive]: new HsvEnumMapping('Rainbow Reactive', 'tt_rainbow_react_hsv'),
		[TurntableMode.RainbowSpin]: new HsvEnumMapping('Rainbow Spin', 'tt_rainbow_spin_hsv'),
		[TurntableMode.Reactive]: new HsvEnumMapping('Reactive', 'tt_react_hsv'),
		[TurntableMode.Breathing]: new HsvEnumMapping('Breathing', 'tt_breathing_hsv'),
		[TurntableMode.HID]: new HsvEnumMapping('HID'),
		[TurntableMode.Off]: new HsvEnumMapping('Off')
	} as const;

	const barModeMapping: Record<BarMode, HsvEnumMapping> = {
		[BarMode.KeySpectrumP1]: new HsvEnumMapping('Key Spectrum (P1)'),
		[BarMode.KeySpectrumP2]: new HsvEnumMapping('Key Spectrum (P2)'),
		[BarMode.HID]: new HsvEnumMapping('HID'),
		[BarMode.Off]: new HsvEnumMapping('Off')
	} as const;

	const controllerTypes = Object.values(ControllerType)
		.filter((v) => isNaN(Number(v)))
		.map((_, index) => ({
			value: index,
			label: ControllerType[index]
		}));
</script>

{#if config}
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
		<WarningAlert
			title="Heads up!"
			description="Controller mode changed. Please replug the controller to apply the new mode."
		/>
	{/if}

	<Separator class="mb-4" />

	{#if config.controller_type === ControllerType.IIDX}
		<div>
			<h3 class="mb-2 text-lg font-semibold">IIDX Configuration</h3>
			<InputModes bind:inputMode={config.iidx_input_mode} />
			<Switch label="Reverse Turntable" bind:checked={config.reverse_tt} />
			<div class="mb-4">
				<ToolTipLabel label="Turntable Deadzone">
					<p>Only affects digital turntable input</p>
				</ToolTipLabel>
				<SliderInput bind:value={config.tt_deadzone} min={1} max={6} id="tt-deadzone" />
			</div>

			{#if config.version >= 12}
				<div class="mb-4">
					<ToolTipLabel label="Turntable Sustain Time">
						<p>Controls how long in milliseconds the last turntable direction is sustained.</p>
						<p>
							It effectively controls how sensitive the digital TT is to when the turntable starts
							and stops spinning.
						</p>
					</ToolTipLabel>
					<SliderInput bind:value={config.tt_sustain_ms} min={0} max={250} id="tt-sustain-ms" />
				</div>
			{/if}

			<div class="mb-4">
				<Label for="tt-ratio">Turntable Sensitivity</Label>
				<!-- We store TT ratio but present it as TT sensitivity, so invert the range -->
				<SliderInput bind:value={config.tt_ratio} min={1} max={6} id="tt-ratio" reversed={true} />
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
		<AlertDialog.Root closeOnOutsideClick={true}>
			<AlertDialog.Trigger asChild let:builder>
				<Button builders={[builder]} variant="destructive">Reset Config</Button>
			</AlertDialog.Trigger>
			<AlertDialog.Content>
				<AlertDialog.Header>
					<AlertDialog.Title>Are you sure?</AlertDialog.Title>
					<AlertDialog.Description>
						This action cannot be undone. This will reset all settings to their default values and
						the controller will disconnect.
					</AlertDialog.Description>
				</AlertDialog.Header>
				<AlertDialog.Footer>
					<AlertDialog.Cancel>Cancel</AlertDialog.Cancel>
					<AlertDialog.Action
						on:click={async () => {
							await sendCommand(Command.ResetConfig);
							await waitForReconnection();
						}}>Continue</AlertDialog.Action
					>
				</AlertDialog.Footer>
			</AlertDialog.Content>
		</AlertDialog.Root>
	</div>
{/if}

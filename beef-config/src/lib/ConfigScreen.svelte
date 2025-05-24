<script lang="ts">
	import { onMount } from 'svelte';

	import * as AlertDialog from '$lib/components/ui/alert-dialog/index.js';
	import { Button } from '$lib/components/ui/button';
	import { Label } from '$lib/components/ui/label/index.js';
	import * as Select from '$lib/components/ui/select';
	import { Separator } from '$lib/components/ui/separator/index.js';

	import InputModes from '$lib/InputModes.svelte';
	import KeyBinding from '$lib/KeyBinding.svelte';
	import LightEffectSelect from '$lib/LightEffectSelect.svelte';
	import SliderInput from '$lib/SliderInput.svelte';
	import Switch from '$lib/Switch.svelte';
	import ToolTipLabel from '$lib/ToolTipLabel.svelte';

	import { readConfig, updateConfig, type Config } from '$lib/types/config.svelte';
	import { Command, sendCommand, waitForReconnection } from '$lib/types/hid';
	import { appState } from '$lib/types/state.svelte';
	import { TurntableMode, BarMode, ControllerType } from '$lib/types/types.svelte';
	import WarningAlert from './WarningAlert.svelte';
	import ColorPicker from './ColorPicker.svelte';

	let config: Config | undefined = $state();
	let controllerTypeChanged = $state(false);

	onMount(async () => {
		try {
			config = await readConfig();
		} catch (err) {
			appState.error = `${err}`;
		}
	});

	$effect(() => {
		if (config) {
			try {
				updateConfig(config);
			} catch (err) {
				appState.error = `${err}`;
			}
		}
	});
</script>

{#if config}
	{#if config.version < 14}
		<WarningAlert
			title="Outdated Firmware"
			description="Your firmware version is too old. Some features may not be available. Please update your firmware to access all features."
		/>
	{/if}

	<div class="mb-4">
		<Label for="controller-type">Controller Mode</Label>
		<Select.Root
			type="single"
			bind:value={config.controller_type}
			onValueChange={() => controllerTypeChanged = true}
		>
			<Select.Trigger class="w-[180px]">{config.controller_type}</Select.Trigger>
			<Select.Content>
				<Select.Group>
					{#each Object.values(ControllerType) as value}
						<Select.Item {value}>{value}</Select.Item>
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
					Only affects digital turntable input
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
				{@const ttModeMapping = Object.values(TurntableMode)}
				<LightEffectSelect
					label="Turntable Effect"
					bind:effect={config.tt_effect}
					modeMapping={ttModeMapping}
				/>
				{#if config.tt_effect === TurntableMode.Static}
					<ColorPicker bind:hsv={config.tt_static_hsv} />
				{:else if config.tt_effect === TurntableMode.Spin}
					<ColorPicker bind:hsv={config.tt_spin_hsv} />
				{:else if config.tt_effect === TurntableMode.Shift}
					<ColorPicker bind:hsv={config.tt_shift_hsv} />
				{:else if config.tt_effect === TurntableMode.RainbowStatic}
					<ColorPicker bind:hsv={config.tt_rainbow_static_hsv} />
				{:else if config.tt_effect === TurntableMode.RainbowReactive}
					<ColorPicker bind:hsv={config.tt_rainbow_react_hsv} />
				{:else if config.tt_effect === TurntableMode.RainbowSpin}
					<ColorPicker bind:hsv={config.tt_rainbow_spin_hsv} />
				{:else if config.tt_effect === TurntableMode.Reactive}
					<ColorPicker bind:hsv={config.tt_react_hsv} />
				{:else if config.tt_effect === TurntableMode.Breathing}
					<ColorPicker bind:hsv={config.tt_breathing_hsv} />
				{/if}

				{@const barModeMapping = Object.values(BarMode)}
				<LightEffectSelect
					label="Light Bar Effect"
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

	{#if config.version >= 13}
		<Separator class="mb-4" />
		<KeyBinding bind:config />
	{/if}

	<Separator class="mb-4" />
{/if}

<div class="mt-4">
	<AlertDialog.Root>
		<AlertDialog.Trigger>
			{#snippet child({ props })}
				<Button {...props} variant="destructive">Reset Config</Button>
			{/snippet}
		</AlertDialog.Trigger>
		<AlertDialog.Content>
			<AlertDialog.Header>
				<AlertDialog.Title>Are you sure?</AlertDialog.Title>
				<AlertDialog.Description>
					This action cannot be undone. This will reset all settings to their default values and the
					controller will disconnect.
				</AlertDialog.Description>
			</AlertDialog.Header>
			<AlertDialog.Footer>
				<AlertDialog.Action
					onclick={async () => {
						await sendCommand(Command.ResetConfig);
						await waitForReconnection();
					}}>Continue</AlertDialog.Action
				>
				<AlertDialog.Cancel>Cancel</AlertDialog.Cancel>
			</AlertDialog.Footer>
		</AlertDialog.Content>
	</AlertDialog.Root>
</div>

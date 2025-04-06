<script lang="ts">
	import { onMount } from 'svelte';
	import { Button } from '$lib/components/ui/button';
	import { Label } from '$lib/components/ui/label';
	import * as AlertDialog from '$lib/components/ui/alert-dialog';
	import { ControllerType } from '$lib/types/types';
	import { IIDXKeyMapping, SDVXKeyMapping, type Config } from '$lib/types/config';
	import { getKeyCode, getKeyName } from '$lib/types/hid-codes';
	import { error } from '$lib/types/state';

	export let config: Config;

	let selectedButton: number | null = null;

	// Button labels
	const IIDX_BUTTON_LABELS = {
		main_buttons: ['1', '2', '3', '4', '5', '6', '7'],
		function_buttons: ['E1', 'E2', 'E3', 'E4'],
		tt_ccw: 'TT-',
		tt_cw: 'TT+'
	};

	const SDVX_BUTTON_LABELS = {
		bt_buttons: ['BT-A', 'BT-B', 'BT-C', 'BT-D'],
		fx_buttons: ['FX-L', 'FX-R'],
		start: 'Start'
	};

	function startKeyCapture(button: number) {
		selectedButton = button;
		window.addEventListener('keydown', handleKeyDown);
	}

	function handleKeyDown(event: KeyboardEvent) {
		if (selectedButton === null) return;

		event.preventDefault();
		window.removeEventListener('keydown', handleKeyDown);

		const hidKeyCode = getKeyCode(event.code);
		if (hidKeyCode) {
			switch (config.controller_type) {
				case ControllerType.IIDX:
					if (selectedButton < 7) {
						config.iidx_keys.main_buttons[selectedButton] = hidKeyCode;
					} else if (selectedButton < 11) {
						config.iidx_keys.function_buttons[selectedButton - 7] = hidKeyCode;
					} else if (selectedButton === 11) {
						config.iidx_keys.tt_ccw = hidKeyCode;
					} else if (selectedButton === 12) {
						config.iidx_keys.tt_cw = hidKeyCode;
					}
					break;
				case ControllerType.SDVX:
					if (selectedButton < 4) {
						config.sdvx_keys.bt_buttons[selectedButton] = hidKeyCode;
					} else if (selectedButton < 6) {
						config.sdvx_keys.fx_buttons[selectedButton - 4] = hidKeyCode;
					} else if (selectedButton === 9) {
						config.sdvx_keys.start = hidKeyCode;
					}
					break;
			}
		} else {
			error.set(`Cannot bind button to ${event.code}`);
		}

		selectedButton = null;
	}

	function resetKeys() {
		switch (config.controller_type) {
			case ControllerType.IIDX:
				config.iidx_keys = new IIDXKeyMapping();
				break;
			case ControllerType.SDVX:
				config.sdvx_keys = new SDVXKeyMapping();
				break;
		}
	}
</script>

<div class="mb-4">
	<div class="mb-2 flex items-center justify-between">
		<h3 class="text-lg font-semibold">Key Bindings</h3>
		<AlertDialog.Root>
			<AlertDialog.Trigger asChild let:builder>
				<Button builders={[builder]} variant="destructive">Reset Keys</Button>
			</AlertDialog.Trigger>
			<AlertDialog.Content>
				<AlertDialog.Header>
					<AlertDialog.Title>Reset Key Bindings?</AlertDialog.Title>
					<AlertDialog.Description>
						This will reset all key bindings to their default values.
					</AlertDialog.Description>
				</AlertDialog.Header>
				<AlertDialog.Footer>
					<AlertDialog.Action on:click={resetKeys}>Reset</AlertDialog.Action>
					<AlertDialog.Cancel>Cancel</AlertDialog.Cancel>
				</AlertDialog.Footer>
			</AlertDialog.Content>
		</AlertDialog.Root>
	</div>

	<div class="flex flex-col">
		{#if config.controller_type === ControllerType.IIDX}
			<div class="mb-2 flex flex-col">
				<Label>Main Buttons</Label>
				<div class="grid grid-cols-4 gap-2">
					{#each IIDX_BUTTON_LABELS.main_buttons as label, i}
						<Button
							variant={selectedButton === i ? 'default' : 'outline'}
							on:click={() => startKeyCapture(i)}
						>
							{label}: {getKeyName(config.iidx_keys.main_buttons[i])}
						</Button>
					{/each}
				</div>
			</div>

			<div class="mb-2 flex flex-col">
				<Label>Function Buttons</Label>
				<div class="grid grid-cols-4 gap-2">
					{#each IIDX_BUTTON_LABELS.function_buttons as label, i}
						<Button
							variant={selectedButton === i + 7 ? 'default' : 'outline'}
							on:click={() => startKeyCapture(i + 7)}
						>
							{label}: {getKeyName(config.iidx_keys.function_buttons[i])}
						</Button>
					{/each}
				</div>
			</div>

			<div class="mb-2 flex flex-col">
				<Label>Turntable</Label>
				<div class="grid grid-cols-2 gap-2">
					<Button
						variant={selectedButton === 11 ? 'default' : 'outline'}
						on:click={() => startKeyCapture(11)}
					>
						{IIDX_BUTTON_LABELS.tt_ccw}: {getKeyName(config.iidx_keys.tt_ccw)}
					</Button>
					<Button
						variant={selectedButton === 12 ? 'default' : 'outline'}
						on:click={() => startKeyCapture(12)}
					>
						{IIDX_BUTTON_LABELS.tt_cw}: {getKeyName(config.iidx_keys.tt_cw)}
					</Button>
				</div>
			</div>
		{:else}
			<div class="mb-2 flex flex-col">
				<Label>BT Buttons</Label>
				<div class="grid grid-cols-2 gap-2">
					{#each SDVX_BUTTON_LABELS.bt_buttons as label, i}
						<Button
							variant={selectedButton === i ? 'default' : 'outline'}
							on:click={() => startKeyCapture(i)}
						>
							{label}: {getKeyName(config.sdvx_keys.bt_buttons[i])}
						</Button>
					{/each}
				</div>
			</div>

			<div class="mb-2 flex flex-col">
				<Label>FX Buttons</Label>
				<div class="grid grid-cols-2 gap-2">
					{#each SDVX_BUTTON_LABELS.fx_buttons as label, i}
						<Button
							variant={selectedButton === i + 4 ? 'default' : 'outline'}
							on:click={() => startKeyCapture(i + 4)}
						>
							{label}: {getKeyName(config.sdvx_keys.fx_buttons[i])}
						</Button>
					{/each}
				</div>
			</div>

			<div class="flex flex-col">
				<Label>Start Button</Label>
				<Button
					variant={selectedButton === 9 ? 'default' : 'outline'}
					on:click={() => startKeyCapture(9)}
				>
					{SDVX_BUTTON_LABELS.start}: {getKeyName(config.sdvx_keys.start)}
				</Button>
			</div>
		{/if}
	</div>

	{#if selectedButton !== null}
		<div class="bg-muted mt-4 rounded-md p-4">Press any key to bind...</div>
	{/if}
</div>

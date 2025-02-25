<script lang="ts">
	import { onMount } from 'svelte';
	import ColorPicker from 'svelte-awesome-color-picker';
	import type { HsvaColor } from 'colord';
	import { mode } from 'mode-watcher';

	import { type Config } from '$lib/types/config';
	import { Hsv } from '$lib/types/types';

	export let config: Config;
	export let hsvField: keyof Config | undefined;

	let hsva = { h: 0, s: 0, v: 0 } as HsvaColor;
	let colourPickerElement: HTMLDivElement;

	onMount(updateHsvBinding);

	$: if (hsvField) {
		updateHsvBinding();
	}

	function updateHsvBinding(): void {
		if (hsvField) {
			const hsv = config[hsvField] as Hsv;
			hsva.h = hsv.h;
			hsva.s = hsv.s;
			hsva.v = hsv.v;
		}
	}

	// Limit updates so that we don't immediately wear out EEPROM if the user spasms out on the colour picker
	function handleHsvStart(event: PointerEvent): void {
		colourPickerElement.setPointerCapture(event.pointerId);
	}

	function handleHsvEnd(event: PointerEvent): void {
		colourPickerElement.releasePointerCapture(event.pointerId);
		handleHsvChange();
	}

	function handleHsvChange(): void {
		if (hsvField) {
			const hsv = config[hsvField];
			if (hsv instanceof Hsv) {
				hsv.fromHsva(hsva);
				// Hack to force an update
				config = config;
			}
		}
	}
</script>

{#if hsvField}
	<div
		class="mb-4"
		class:cp-dark={$mode === 'dark'}
		bind:this={colourPickerElement}
		role="none"
		on:pointerdown={handleHsvStart}
		on:pointerup={handleHsvEnd}
		on:input={handleHsvChange}
	>
		<ColorPicker position="responsive" isAlpha={false} on:input bind:hsv={hsva} />
	</div>
{/if}

<style>
	.cp-dark {
		/* Comes from the dark class in app.css */
		--cp-bg-color: hsl(222.2 84% 4.9%);
		--cp-border-color: hsl(217.2 32.6% 17.5%);
		--cp-text-color: 217.2 32.6% 17.5%;
		--cp-input-color: hsl(222.2 47.4% 11.2%);
		--cp-button-hover-color: hsl(217.2 32.6% 17.5%);
	}
</style>

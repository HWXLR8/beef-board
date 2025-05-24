<script lang="ts">
	import ColorPicker from 'svelte-awesome-color-picker';
	import { mode } from 'mode-watcher';

	import { Hsv } from '$lib/types/types.svelte';

	interface Props {
		hsv: Hsv;
	}

	let { hsv = $bindable() }: Props = $props();

	let colourPickerElement: HTMLDivElement | undefined = $state();
	let hsva = $state({
		h: hsv.h,
		s: hsv.s,
		v: hsv.v,
		a: 1
	});

	// Limit updates so that we don't immediately wear out EEPROM if the user spasms out on the colour picker
	function handleHsvStart(event: PointerEvent): void {
		colourPickerElement?.setPointerCapture(event.pointerId);
	}

	function handleHsvEnd(event: PointerEvent): void {
		colourPickerElement?.releasePointerCapture(event.pointerId);
		hsv.h = hsva.h;
		hsv.s = hsva.s;
		hsv.v = hsva.v;
	}
</script>

<div
	class="mb-4"
	class:cp-dark={mode.current === 'dark'}
	bind:this={colourPickerElement}
	role="none"
	onpointerdown={handleHsvStart}
	onpointerup={handleHsvEnd}
>
	<ColorPicker position="responsive" isAlpha={false} bind:hsv={hsva} />
</div>

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

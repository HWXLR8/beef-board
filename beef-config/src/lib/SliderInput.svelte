<script lang="ts">
	import Input from '$lib/components/ui/input/input.svelte';
	import Slider from '$lib/components/ui/slider/slider.svelte';

	export let value: number;
	export let min: number;
	export let max: number;
	export let id: string;
	export let reversed = false;

	let displayedValue = reversed ? max + 1 - value : value;
	$: sliderValue = [displayedValue];
	$: value = reversed ? max + 1 - displayedValue : displayedValue;
</script>

<div class="flex items-center space-x-6">
	<Slider
		{id}
		{min}
		{max}
		step={1}
		bind:value={sliderValue}
		onValueChange={(v) => {
			displayedValue = v[0];
		}}
	/>
	<Input class="w-1/5" {id} {min} {max} type="number" bind:value={displayedValue} />
</div>

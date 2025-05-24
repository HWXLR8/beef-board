<script lang="ts">
	import { Input } from '$lib/components/ui/input';
	import { Slider } from '$lib/components/ui/slider';

	interface Props {
		value: number;
		min: number;
		max: number;
		id: string;
		reversed?: boolean;
	}

	let { value = $bindable(), min, max, id, reversed = false }: Props = $props();

	let displayedValue = $state(reversed ? max + 1 - value : value);

	$effect(() => {
		value = reversed ? max + 1 - displayedValue : displayedValue;
	});
</script>

<div class="flex items-center space-x-6">
	<Slider
		type="single"
		{id}
		{min}
		{max}
		value={displayedValue}
		onValueCommit={(v) => (displayedValue = v)}
	/>
	<Input class="w-1/5" {id} {min} {max} type="number" bind:value={displayedValue} />
</div>

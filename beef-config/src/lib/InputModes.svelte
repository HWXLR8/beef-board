<script lang="ts">
	import { Label } from '$lib/components/ui/label';
	import * as Select from '$lib/components/ui/select';

	import { InputMode } from '$lib/types/types.svelte';

	interface Props {
		inputMode: InputMode;
	}

	let { inputMode = $bindable() }: Props = $props();
</script>

<div class="mb-4">
	<Label>Input Mode</Label>
	<Select.Root
		type="single"
		bind:value={inputMode}
	>
		<Select.Trigger class="w-[180px]">{inputMode}</Select.Trigger>
		<Select.Content>
			<Select.Group>
				{#each Object.values(InputMode) as value}
					<Select.Item {value}>{value}</Select.Item>
				{/each}
			</Select.Group>
		</Select.Content>
	</Select.Root>
</div>

<script lang="ts" generics="T extends TurntableMode | BarMode">
	import { Label } from '$lib/components/ui/label';
	import * as Select from '$lib/components/ui/select';

	import type { BarMode, TurntableMode } from '$lib/types/types.svelte';

	interface Props {
		label: string;
		effect: T;
		modeMapping: T[];
	}

	let { label, effect = $bindable(), modeMapping }: Props = $props();
</script>

<div class="mb-4">
	<Label>{label}</Label>
	<Select.Root
		type="single"
		bind:value={effect}
	>
		<Select.Trigger class="w-[180px]">{effect}</Select.Trigger>
		<Select.Content>
			<Select.Group>
				{#each modeMapping as value}
					<Select.Item {value}>{value}</Select.Item>
				{/each}
			</Select.Group>
		</Select.Content>
	</Select.Root>
</div>

<script lang="ts">
	import { Label } from '$lib/components/ui/label';
	import * as Select from '$lib/components/ui/select';

	import { InputMode } from '$lib/types/types';

	export let inputMode: InputMode;

	const inputModes = Object.values(InputMode)
		.filter((v) => isNaN(Number(v)))
		.map((_, index) => ({
			value: index,
			label: InputMode[index]
		}));
</script>

<div class="mb-4">
	<Label>Input Mode</Label>
	<Select.Root
		selected={{
			value: inputMode,
			label: InputMode[inputMode]
		}}
		onSelectedChange={(value) => {
			if (value) {
				inputMode = value.value;
			}
		}}
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

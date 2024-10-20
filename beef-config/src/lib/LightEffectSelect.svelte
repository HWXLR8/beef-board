<script lang="ts" extends generics="T extends TurntableMode | BarMode">
	import { Label } from '$lib/components/ui/label';
	import * as Select from '$lib/components/ui/select';

	import ColorPicker from '$lib/ColorPicker.svelte';
	import type { Config } from '$lib/types/config';
	import type { BarMode, EnumMapping, TurntableMode } from '$lib/types/types';

	export let label: string;
	export let config: Config;
	export let effect: T;
	export let modeMapping: Record<T, EnumMapping>;

	const modeMappingIter = Object.entries<EnumMapping>(modeMapping);
</script>

<div class="mb-4">
	<Label>{label}</Label>
	<Select.Root
		selected={{
			value: effect,
			label: modeMapping[effect].displayName
		}}
		onSelectedChange={(value) => {
			if (value) {
				effect = value.value;
			}
		}}
	>
		<Select.Trigger class="w-[180px]">
			<Select.Value placeholder={label} />
		</Select.Trigger>
		<Select.Content>
			<Select.Group>
				{#each modeMappingIter as [value, label]}
					<Select.Item value={Number(value)} label={label.displayName}
						>{label.displayName}</Select.Item
					>
				{/each}
			</Select.Group>
		</Select.Content>
	</Select.Root>
</div>
<ColorPicker bind:config hsvField={modeMapping[effect].hsvField} />

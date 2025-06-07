<script lang="ts">
	import { CircleHelp } from '@lucide/svelte';

	import { Label } from '$lib/components/ui/label';
	import * as Tooltip from '$lib/components/ui/tooltip';

	interface Props {
		label: string;
		children?: import('svelte').Snippet;
	}

	let { label, children }: Props = $props();
</script>

<div class="flex items-center space-x-1">
	<Label>{label}</Label>
	<Tooltip.Provider>
		<Tooltip.Root>
			<Tooltip.Trigger>
				<CircleHelp />
			</Tooltip.Trigger>
			<Tooltip.Content>
				{@render children?.()}
			</Tooltip.Content>
		</Tooltip.Root>
	</Tooltip.Provider>
</div>

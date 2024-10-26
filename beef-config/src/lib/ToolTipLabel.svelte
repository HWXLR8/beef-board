<script lang="ts">
	import { CircleHelp } from 'lucide-svelte';

	import { Label } from '$lib/components/ui/label';
	import * as Tooltip from '$lib/components/ui/tooltip/index.js';

	export let label: string;
</script>

<div class="flex items-center space-x-1">
	<Label>{label}</Label>
	<Tooltip.Root>
		<Tooltip.Trigger>
			<CircleHelp />
		</Tooltip.Trigger>
		<Tooltip.Content>
			<slot />
		</Tooltip.Content>
	</Tooltip.Root>
</div>

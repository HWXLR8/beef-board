<script lang="ts">
	import { Label } from '$lib/components/ui/label';
	import { Switch } from '$lib/components/ui/switch';

	export let label: string;
	export let checked: boolean;
</script>

<div class="mb-4 flex items-center space-x-2">
	<Label for={label}>{label}</Label>
	<Switch id={label} bind:checked />
</div>

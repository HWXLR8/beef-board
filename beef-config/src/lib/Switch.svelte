<script lang="ts">
	import { Label } from '$lib/components/ui/label';
	import { Switch } from '$lib/components/ui/switch';

	interface Props {
		label: string;
		checked: boolean;
	}

	let { label, checked = $bindable() }: Props = $props();
</script>

<div class="mb-4 flex items-center space-x-2">
	<Label for={label}>{label}</Label>
	<Switch id={label} bind:checked />
</div>

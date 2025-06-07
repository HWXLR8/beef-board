<script lang="ts">
	import { TriangleAlert } from '@lucide/svelte';
	import { Alert, AlertDescription, AlertTitle } from '$lib/components/ui/alert';

	interface Props {
		title: string;
		description: string;
		variant?: 'default' | 'destructive';
	}

	let { title, description, variant = 'default' }: Props = $props();
</script>

<Alert class="mb-4" {variant}>
	<TriangleAlert class="h-4 w-4" />
	<AlertTitle>{title}</AlertTitle>
	<AlertDescription>{description}</AlertDescription>
</Alert>

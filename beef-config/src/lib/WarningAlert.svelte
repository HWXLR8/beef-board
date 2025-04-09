<script lang="ts">
	import { TriangleAlert } from 'lucide-svelte';
	import { Alert } from '$lib/components/ui/alert';
	import AlertDescription from '$lib/components/ui/alert/alert-description.svelte';
	import AlertTitle from '$lib/components/ui/alert/alert-title.svelte';

	export let title: string;
	export let description: string;
	export let variant: 'default' | 'destructive' = 'default';
</script>

<Alert class="mb-4" {variant}>
	<TriangleAlert class="h-4 w-4" />
	<AlertTitle>{title}</AlertTitle>
	<AlertDescription>{description}</AlertDescription>
</Alert>

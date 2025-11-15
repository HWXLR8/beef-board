<script lang="ts">
	import { onMount } from 'svelte';

	import { Alert, AlertDescription, AlertTitle } from '$lib/components/ui/alert';
	import { Button } from '$lib/components/ui/button';
	import { Skeleton } from '$lib/components/ui/skeleton';
	import * as Tabs from '$lib/components/ui/tabs';

	import ConfigScreen from '$lib/ConfigScreen.svelte';
	import FirmwareScreen from '$lib/FirmwareScreen.svelte';
	import LightDarkModeToggle from '$lib//LightDarkModeToggle.svelte';

	import { connectDevice, appState } from '$lib/types/state.svelte';

	const Tab = {
		Config: 'config',
		Firmware: 'dfu'
	} as const;

	let browserSupported = $state(false);
	let loading = $state(true);

	onMount(() => {
		checkBrowserSupport();
		loading = false;
	});

	function checkBrowserSupport(): void {
		browserSupported = 'hid' in navigator && 'usb' in navigator;
	}
</script>

<main class="mx-auto max-w-(--breakpoint-lg) p-4">
	<title>Beef Board Configurator</title>
	<div class="flex justify-between">
		<h1 class="mb-4 text-2xl font-bold">Beef Board Configurator</h1>
		<LightDarkModeToggle />
	</div>

	{#if loading}
		<Skeleton class="h-10" />
	{:else if !browserSupported}
		<Alert variant="destructive">
			<AlertTitle>Browser Not Supported</AlertTitle>
			<AlertDescription>
				WebHID/WebUSB is not supported in this browser. Please use a compatible browser like Chrome
				or Edge.
			</AlertDescription>
		</Alert>
	{:else}
		{#if !appState.device}
			<Button onclick={connectDevice} class="mb-4">Connect Device</Button>
		{:else}
			<Tabs.Root value={Tab.Config}>
				<Tabs.List class="mb-4 flex w-full flex-row justify-center">
					<Tabs.Trigger value={Tab.Config} disabled={appState.disableConfigTab} class="grow"
						>Config</Tabs.Trigger
					>
					<Tabs.Trigger value={Tab.Firmware} class="grow">Firmware</Tabs.Trigger>
				</Tabs.List>
				<Tabs.Content value={Tab.Config}><ConfigScreen /></Tabs.Content>
				<Tabs.Content value={Tab.Firmware}><FirmwareScreen /></Tabs.Content>
			</Tabs.Root>
		{/if}

		{#if appState.error}
			<Alert variant="destructive" class="mt-4">
				<AlertTitle>Error</AlertTitle>
				<AlertDescription>{appState.error}</AlertDescription>
			</Alert>
		{/if}
	{/if}
</main>

<script lang="ts">
	import { onMount } from 'svelte';

	import { Alert } from '$lib/components/ui/alert';
	import AlertDescription from '$lib/components/ui/alert/alert-description.svelte';
	import AlertTitle from '$lib/components/ui/alert/alert-title.svelte';
	import { Button } from '$lib/components/ui/button';
	import { Skeleton } from '$lib/components/ui/skeleton/index.js';
	import * as Tabs from '$lib/components/ui/tabs';

	import ConfigScreen from '$lib/ConfigScreen.svelte';
	import FirmwareScreen from '$lib/FirmwareScreen.svelte';
	import LightDarkModeToggle from '$lib//LightDarkModeToggle.svelte';

	import { connectDevice, disableConfigTab, device, error } from '$lib/types/state';

	const Tab = {
		Config: 'config',
		Firmware: 'dfu'
	} as const;

	let browserSupported = false;
	let loading = true;

	onMount(() => {
		checkBrowserSupport();
		loading = false;
	});

	function checkBrowserSupport(): void {
		browserSupported = 'hid' in navigator && 'usb' in navigator;
	}
</script>

<main class="mx-auto max-w-screen-lg p-4">
	<div class="flex justify-between">
		<h1 class="mb-4 text-2xl font-bold">Beef Board Configuration</h1>
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
		{#if !$device}
			<Button on:click={connectDevice} class="mb-4">Connect Device</Button>
		{:else}
			<Tabs.Root value={Tab.Config}>
				<Tabs.List class="mb-4 flex w-full flex-row justify-center">
					<Tabs.Trigger value={Tab.Config} disabled={$disableConfigTab} class="grow"
						>Config</Tabs.Trigger
					>
					<Tabs.Trigger value={Tab.Firmware} class="grow">Firmware</Tabs.Trigger>
				</Tabs.List>
				<Tabs.Content value={Tab.Config}><ConfigScreen /></Tabs.Content>
				<Tabs.Content value={Tab.Firmware}><FirmwareScreen /></Tabs.Content>
			</Tabs.Root>
		{/if}

		{#if $error}
			<Alert variant="destructive" class="mb-4">
				<AlertTitle>Error</AlertTitle>
				<AlertDescription>{$error}</AlertDescription>
			</Alert>
		{/if}
	{/if}
</main>

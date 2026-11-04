// Copyright Vaslabs LTD 2025 All Rights Reserved.

#include "SteamInputSimpleBlueprintLibrary.h"
#include "SteamInputSubsystem.h"
#include "SteamInputSimpleInternal.h"

TArray<FSteamControllerInfo> USteamInputSimpleBlueprintLibrary::GetConnectedControllers()
{
	const USteamInputSubsystem* Subsystem = USteamInputSubsystem::Get();
	return Subsystem ? Subsystem->GetConnectedControllers() : TArray<FSteamControllerInfo>();
}

void USteamInputSimpleBlueprintLibrary::OnSteamOverlayStateChanged(const FOnSteamOverlayActivatedDelegate OnOverlayActivatedDelegate)
{
	USteamInputSubsystem* Subsystem = USteamInputSubsystem::Get();
	if (!Subsystem)
	{
		UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("Steam Input subsystem is not available."));
		return;
	}

	// Keep the old single-binding semantics: replace whatever was bound through this function before.
	static FOnSteamOverlayActivatedDelegate PreviousBinding;
	if (PreviousBinding.IsBound())
	{
		Subsystem->OnOverlayActivated.Remove(PreviousBinding);
	}
	PreviousBinding = OnOverlayActivatedDelegate;
	Subsystem->OnOverlayActivated.AddUnique(OnOverlayActivatedDelegate);
}

void USteamInputSimpleBlueprintLibrary::RunSteamCallbacks()
{
	// No-op: USteamInputSubsystem pumps SteamAPI_RunCallbacks every frame.
}

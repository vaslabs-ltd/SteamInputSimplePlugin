// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPauseSubsystem.h"
#include "SteamInputPromptSubsystem.h"
#include "SteamInputSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

#define LOCTEXT_NAMESPACE "SteamInputPause"

DEFINE_LOG_CATEGORY_STATIC(LogSteamInputAutoPause, Log, All);

void USteamInputPauseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DeviceConnectionHandle = IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddUObject(this, &USteamInputPauseSubsystem::HandleDeviceConnectionChange);
	if (USteamInputSubsystem* Steam = USteamInputSubsystem::Get())
	{
		OverlayHandle = Steam->OnOverlayActivatedNative.AddUObject(this, &USteamInputPauseSubsystem::HandleOverlay);
	}
}

void USteamInputPauseSubsystem::Deinitialize()
{
	IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().Remove(DeviceConnectionHandle);
	if (USteamInputSubsystem* Steam = USteamInputSubsystem::Get())
	{
		Steam->OnOverlayActivatedNative.Remove(OverlayHandle);
	}
	Super::Deinitialize();
}

void USteamInputPauseSubsystem::SetMatchInProgress(bool bInProgress)
{
	bMatchInProgress = bInProgress;
	if (!bInProgress)
	{
		MissingUsers.Reset();
		bAutoPaused = false;
	}
}

int32 USteamInputPauseSubsystem::FindLocalPlayerIndex(FPlatformUserId UserId) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return INDEX_NONE;
	}
	const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
	for (int32 Index = 0; Index < LocalPlayers.Num(); ++Index)
	{
		if (LocalPlayers[Index] && LocalPlayers[Index]->GetPlatformUserId() == UserId)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

void USteamInputPauseSubsystem::HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	const int32 PlayerIndex = FindLocalPlayerIndex(UserId);
	UE_LOG(LogSteamInputAutoPause, Log, TEXT("Device %d %s for user %d (local player %d), match in progress: %d"), DeviceId.GetId(),
		NewState == EInputDeviceConnectionState::Connected ? TEXT("connected") : TEXT("disconnected"), UserId.GetInternalId(), PlayerIndex, bMatchInProgress);

	if (NewState == EInputDeviceConnectionState::Connected)
	{
		if (MissingUsers.Remove(UserId) > 0)
		{
			OnPauseStateChanged.Broadcast();
		}
		return;
	}

	if (NewState != EInputDeviceConnectionState::Disconnected || !bMatchInProgress || PlayerIndex == INDEX_NONE)
	{
		return;
	}
	// The player may still have another pad (or the keyboard) mapped.
	TArray<FInputDeviceId> Devices;
	IPlatformInputDeviceMapper::Get().GetAllConnectedInputDevicesForUser(UserId, Devices);
	Devices.Remove(IPlatformInputDeviceMapper::Get().GetDefaultInputDevice());
	if (Devices.Num() > 0)
	{
		return;
	}
	MissingUsers.AddUnique(UserId);
	bAutoPaused = true;
	OnAutoPauseRequested.Broadcast(ESteamInputAutoPauseReason::ControllerDisconnected, PlayerIndex);
	OnPauseStateChanged.Broadcast();
}

void USteamInputPauseSubsystem::HandleOverlay(bool bActive)
{
	if (bActive && bMatchInProgress)
	{
		bAutoPaused = true;
		OnAutoPauseRequested.Broadcast(ESteamInputAutoPauseReason::SteamOverlay, INDEX_NONE);
		OnPauseStateChanged.Broadcast();
	}
}

FText USteamInputPauseSubsystem::GetPauseMessage() const
{
	if (!MissingUsers.IsEmpty())
	{
		TArray<FString> Names;
		for (const FPlatformUserId& UserId : MissingUsers)
		{
			Names.Add(FString::Printf(TEXT("Player %d"), FindLocalPlayerIndex(UserId) + 1));
		}
		return FText::Format(LOCTEXT("Reconnect", "Reconnect the controller for {0}"), FText::FromString(FString::Join(Names, TEXT(", "))));
	}
	if (bAutoPaused)
	{
		const USteamInputPromptSubsystem* Controllers = USteamInputPromptSubsystem::Get(GetGameInstance());
		const FText Template = LOCTEXT("Resume", "Press {Start} to resume");
		return Controllers ? Controllers->FormatPromptFor(Template, nullptr) : USteamInputPromptSubsystem::FormatPrompt(Template, ESteamInputGlyphFamily::Xbox);
	}
	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE

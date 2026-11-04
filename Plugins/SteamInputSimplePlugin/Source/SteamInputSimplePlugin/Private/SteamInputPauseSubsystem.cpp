// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPauseSubsystem.h"
#include "SteamInputSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamInputPause, Log, All);

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

void USteamInputPauseSubsystem::HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	if (NewState != EInputDeviceConnectionState::Disconnected)
	{
		return;
	}
	const UGameInstance* GameInstance = GetGameInstance();
	const TArray<ULocalPlayer*>& LocalPlayers = GameInstance ? GameInstance->GetLocalPlayers() : TArray<ULocalPlayer*>();
	const int32 PlayerIndex = LocalPlayers.IndexOfByPredicate([UserId](const ULocalPlayer* Player) { return Player && Player->GetPlatformUserId() == UserId; });
	UE_LOG(LogSteamInputPause, Log, TEXT("Device %d disconnected (user %d, local player %d)"), DeviceId.GetId(), UserId.GetInternalId(), PlayerIndex);
	if (PlayerIndex != INDEX_NONE)
	{
		OnPauseRequested.Broadcast(ESteamInputAutoPauseReason::ControllerDisconnected, PlayerIndex);
	}
}

void USteamInputPauseSubsystem::HandleOverlay(bool bActive)
{
	if (bActive)
	{
		OnPauseRequested.Broadcast(ESteamInputAutoPauseReason::SteamOverlay, INDEX_NONE);
	}
}

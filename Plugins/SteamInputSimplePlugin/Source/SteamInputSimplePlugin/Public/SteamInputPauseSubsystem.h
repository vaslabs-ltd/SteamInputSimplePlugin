// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SteamInputPromptTypes.h"
#include "SteamInputPauseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSteamInputPauseRequested, ESteamInputAutoPauseReason, Reason, int32, PlayerIndex);

/**
 * Tells the game when it should pause: a local player's controller disconnected, or the Steam overlay opened.
 * The game decides what to do with it (e.g. run the same path as the Pause button).
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputPauseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** PlayerIndex: the local player whose controller went away (0 = Player 1), or -1 for the overlay. */
	UPROPERTY(BlueprintAssignable, Category = "Steam Input|Pause")
	FOnSteamInputPauseRequested OnPauseRequested;

private:
	void HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId);
	void HandleOverlay(bool bActive);

	FDelegateHandle DeviceConnectionHandle;
	FDelegateHandle OverlayHandle;
};

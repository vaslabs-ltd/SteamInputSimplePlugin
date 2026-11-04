// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SteamInputPromptTypes.h"
#include "SteamInputPauseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSteamInputAutoPauseRequested, ESteamInputAutoPauseReason, Reason, int32, PlayerIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSteamInputPauseStateChanged);

/**
 * Asks the game manager to pause when a joined player's controller disconnects or the Steam overlay opens,
 * while a match is in progress. Resuming stays manual (Start), and is blocked until every missing controller is back.
 *
 * The game owns the actual pausing: call Set Match In Progress around gameplay, bind On Auto Pause Requested to the
 * pause path, check Can Resume before unpausing and call Notify Resumed after.
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputPauseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Auto-pause only happens while this is set (gameplay; not menus, join screens or party screens). */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Auto Pause")
	void SetMatchInProgress(bool bInProgress);

	UFUNCTION(BlueprintPure, Category = "Steam Input|Auto Pause")
	bool IsMatchInProgress() const { return bMatchInProgress; }

	/** Call when the game unpauses, to clear the auto-pause message. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Auto Pause")
	void NotifyResumed() { bAutoPaused = false; }

	/** False while a joined player's controller is still disconnected. */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Auto Pause")
	bool CanResume() const { return MissingUsers.IsEmpty(); }

	/** Rich text for the pause screen: who to reconnect, or "Press {Start} to resume". Empty when there is nothing to say. */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Auto Pause")
	FText GetPauseMessage() const;

	/** PlayerIndex is the local player's index (0 = Player 1), or -1 for the overlay. */
	UPROPERTY(BlueprintAssignable, Category = "Steam Input|Auto Pause")
	FOnSteamInputAutoPauseRequested OnAutoPauseRequested;

	/** A controller came back (or another went missing): refresh the pause message. */
	UPROPERTY(BlueprintAssignable, Category = "Steam Input|Auto Pause")
	FOnSteamInputPauseStateChanged OnPauseStateChanged;

private:
	void HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId);
	void HandleOverlay(bool bActive);
	int32 FindLocalPlayerIndex(FPlatformUserId UserId) const;

	TArray<FPlatformUserId> MissingUsers;
	bool bMatchInProgress = false;
	bool bAutoPaused = false;
	FDelegateHandle DeviceConnectionHandle;
	FDelegateHandle OverlayHandle;
};

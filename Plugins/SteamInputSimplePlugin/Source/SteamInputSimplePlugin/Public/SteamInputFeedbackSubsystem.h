// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Templates/PimplPtr.h"
#include "SteamInputTypes.h"
#include "SteamInputFeedbackSubsystem.generated.h"

class APlayerController;
class UWorld;
namespace SteamInputSimple { struct FPadFeedback; struct FPadFeedbackMap; }

/**
 * DualSense adaptive trigger effects and DualShock 4 / DualSense lightbar colours per pad, through Steam Input, with
 * the bookkeeping a game needs:
 * - Timed effects (e.g. lock the trigger for a cooldown) that fall back to the pad's lasting effect when they end.
 * - Only changes are sent to Steam.
 * - Triggers are released while the game is paused or the Steam overlay is open (Release Triggers While Paused), and
 *   reset on level change and shutdown, so a stiff trigger never leaks into a menu.
 * - Effects and colours are re-sent when pads reconnect; lightbars go back to the player's Steam colour on exit.
 *
 * Pads are keyed by Steam's gamepad index, which is the local player's ControllerId. Blueprints: use the
 * Steam Input Feedback functions (USteamInputFeedbackLibrary), which take a Player Controller.
 * Other pads, keyboard players and games without Steam: everything is a safe no-op.
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputFeedbackSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USteamInputFeedbackSubsystem();
	virtual ~USteamInputFeedbackSubsystem() override;

	static USteamInputFeedbackSubsystem* Get(const UObject* WorldContext);

	/** Steam's gamepad index for a local player's pad (its ControllerId), or -1 for remote players / null. */
	static int32 GetGamepadIndex(const APlayerController* Player);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Duration <= 0: a lasting effect, kept until replaced or reset (also cancels a running timed effect on that trigger).
	 * Duration > 0: a timed effect in seconds; when it ends the trigger goes back to its lasting effect (Off by default).
	 */
	void SetTriggerEffect(int32 GamepadIndex, ESteamDualSenseTrigger Trigger, const FSteamDualSenseTriggerEffect& Effect, float Duration = 0.f);

	/** Both triggers of one pad back to Off, timed effects included. */
	void ResetTriggerEffects(int32 GamepadIndex);

	/** Every pad's triggers back to Off. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback")
	void ResetAllTriggerEffects();

	/**
	 * Lightbar colour (DualShock 4 / DualSense). Duration <= 0: lasting, until replaced or reset (kept across levels).
	 * Duration > 0: a flash in seconds, then back to the lasting colour (or the player's Steam default).
	 */
	void SetLightbarColor(int32 GamepadIndex, FLinearColor Color, float Duration = 0.f);

	/** Back to the colour the player chose in Steam, flashes included. */
	void ResetLightbar(int32 GamepadIndex);

	/** Every pad's lightbar back to its Steam default. Done automatically on exit. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback")
	void ResetAllLightbars();

	/**
	 * Release the triggers (send Off) while the game is paused or the Steam overlay is open, and restore them after.
	 * Timed effects don't count down meanwhile.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Input|Feedback")
	bool bReleaseTriggersWhilePaused = true;

private:
	bool Tick(float DeltaTime);
	bool IsSuspended() const;
	void Send(int32 GamepadIndex, SteamInputSimple::FPadFeedback& Pad, bool bSuspended);
	void RemoveIfIdle(int32 GamepadIndex);
	void HandleControllersChanged();
	/** Re-send everything now and again shortly after: Steam re-applies its own pad settings when it switches configs. */
	void ScheduleResync();
	void HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId);
	void HandleOverlay(bool bActive);
	void HandleAppActivation(bool bActive);
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	/** Gamepad index -> trigger state. TPimplPtr keeps the plain C++ state out of this header (and UHT's code). */
	TPimplPtr<SteamInputSimple::FPadFeedbackMap> Pads;
	FTSTicker::FDelegateHandle TickHandle;
	FDelegateHandle ControllersChangedHandle;
	FDelegateHandle WorldCleanupHandle;
	FDelegateHandle DeviceConnectionHandle;
	FDelegateHandle OverlayHandle;
	FDelegateHandle ReactivatedHandle;
	/** Seconds until each pending re-send. */
	TArray<float> ResyncDelays;
	bool bWasSuspended = false;
};

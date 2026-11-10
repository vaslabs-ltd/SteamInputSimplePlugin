// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamInputTypes.h"
#include "SteamInputFeedbackLibrary.generated.h"

class APlayerController;

/**
 * DualSense adaptive triggers and DualShock 4 / DualSense lightbars per player, through Steam Input (USteamInputFeedbackSubsystem does the work).
 * Safe to call for every player: pads without adaptive triggers, keyboard players, remote players and games without
 * Steam do nothing. Triggers are released automatically while paused, on level change and on exit.
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputFeedbackLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Applies an adaptive trigger effect to Player's pad.
	 * Duration 0: lasting, until replaced or reset. Duration > 0 (seconds): timed, e.g. Locked for a cooldown; when it
	 * ends the trigger goes back to its lasting effect (Off unless you set one).
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback", meta = (AdvancedDisplay = "Duration"))
	static void SetTriggerEffect(APlayerController* Player, ESteamDualSenseTrigger Trigger, FSteamDualSenseTriggerEffect Effect, float Duration = 0.f);

	/** SetTriggerEffect with a ready-made effect (Light, Heavy, Locked, Weapon, Ramp, Vibration). */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback")
	static void SetTriggerPreset(APlayerController* Player, ESteamDualSenseTrigger Trigger, ESteamDualSenseTriggerPreset Preset, float Duration = 0.f);

	/** The effect behind a preset, to tweak (e.g. a lighter Weapon click) before Set Trigger Effect. */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Feedback")
	static FSteamDualSenseTriggerEffect MakeTriggerEffectFromPreset(ESteamDualSenseTriggerPreset Preset);

	/** Both of Player's triggers back to Off, timed effects included. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback")
	static void ResetTriggerEffects(APlayerController* Player);

	/** Every local player's triggers back to Off (e.g. when a match ends). */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback", meta = (WorldContext = "WorldContext"))
	static void ResetAllTriggerEffects(const UObject* WorldContext);

	/** True when Player holds a DualSense through Steam Input. */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Feedback")
	static bool HasAdaptiveTriggers(APlayerController* Player);

	/**
	 * Lightbar colour of Player's pad (DualShock 4 / DualSense; other pads ignore it).
	 * Duration 0: lasting, until replaced or reset; it stays across levels. Duration > 0 (seconds): a flash (e.g. on a
	 * hit), then back to the lasting colour, or to the player's Steam colour if none was set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback", meta = (AdvancedDisplay = "Duration"))
	static void SetLightbarColor(APlayerController* Player, FLinearColor Color, float Duration = 0.f);

	/** Player's lightbar back to the colour they chose in Steam. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback")
	static void ResetLightbar(APlayerController* Player);

	/** Every local player's lightbar back to their Steam colour. Done automatically on exit. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Feedback", meta = (WorldContext = "WorldContext"))
	static void ResetAllLightbars(const UObject* WorldContext);

	/** True when Player's pad has a lightbar (DualShock 4 or DualSense through Steam Input). */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Feedback")
	static bool HasLightbar(APlayerController* Player);

	/**
	 * Steam's gamepad index for Player's pad (the local player's ControllerId), -1 for remote players. Use it with the
	 * Steam Input Subsystem's index-based calls (e.g. Trigger Vibration).
	 */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Feedback")
	static int32 GetSteamGamepadIndex(APlayerController* Player);
};

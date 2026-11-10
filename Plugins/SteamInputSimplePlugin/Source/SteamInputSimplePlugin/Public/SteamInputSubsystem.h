// Copyright Vaslabs LTD 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Ticker.h"
#include "Templates/PimplPtr.h"
#include "SteamInputTypes.h"
#include "SteamInputSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamInputControllerConnected, const FSteamControllerInfo&, Controller);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamInputControllerDisconnected, const FSteamControllerInfo&, Controller);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamInputOverlayActivated, bool, bActive);

/**
 * Keeps Steam Input alive for the whole session, pumps Steam callbacks every frame and
 * broadcasts controller connect/disconnect and overlay events.
 *
 * Gameplay input still arrives through UE's normal XInput/SDL path (Steam presents every pad as an
 * emulated gamepad); this subsystem tells you what the real controller behind each emulated
 * gamepad index is, and drives the extras (rumble, lightbar, DualSense adaptive triggers).
 *
 * Everything is a safe no-op when Steam is not running.
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	USteamInputSubsystem();
	virtual ~USteamInputSubsystem() override;

	/** Convenience accessor; null when GEngine is not up yet. */
	static USteamInputSubsystem* Get();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** True when Steam is running and Steam Input initialised. */
	UFUNCTION(BlueprintPure, Category = "Simple Steam Input")
	bool IsSteamInputAvailable() const { return bSteamInputReady; }

	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	TArray<FSteamControllerInfo> GetConnectedControllers() const;

	/**
	 * Type of the real controller behind an emulated gamepad index (the XInput slot on Windows,
	 * which matches UE's gamepad ControllerId). Unknown if there is none or Steam is not running.
	 */
	UFUNCTION(BlueprintPure, Category = "Simple Steam Input")
	ESteamControllerType GetControllerTypeForGamepadIndex(int32 GamepadIndex) const;

	UFUNCTION(BlueprintPure, Category = "Simple Steam Input")
	int32 GetGamepadIndexForHandle(int64 Handle) const;

	/** Steam's handle for the pad at an emulated gamepad index, 0 when there is none (yet) or Steam is not running. */
	UFUNCTION(BlueprintPure, Category = "Simple Steam Input")
	int64 GetHandleForGamepadIndex(int32 GamepadIndex) const { return static_cast<int64>(HandleForGamepadIndex(GamepadIndex)); }

	/**
	 * Switch pads: true when Steam maps the face buttons by label (Xbox A = the button labelled A), false when by
	 * position (Xbox A = the bottom button, labelled B). True for other pads and without Steam.
	 */
	UFUNCTION(BlueprintPure, Category = "Simple Steam Input")
	bool UsesNintendoButtonLabels(int32 GamepadIndex) const;

	UFUNCTION(BlueprintPure, Category = "Simple Steam Input")
	bool IsOverlayActive() const { return bOverlayActive; }

	/** Rumble, 0..1 per motor. Normal UE force feedback also works through the emulated gamepad. */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input|Feedback")
	void TriggerVibration(int32 GamepadIndex, float LeftMotor, float RightMotor, float LeftTrigger = 0.f, float RightTrigger = 0.f);

	/** Lightbar colour on DualShock 4 / DualSense. Pass bReset to restore Steam's default colour. */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input|Feedback")
	void SetLightbarColor(int32 GamepadIndex, FLinearColor Color, bool bReset = false);

	/** True when the pad at GamepadIndex is a DualSense (the only pad with adaptive triggers). */
	UFUNCTION(BlueprintPure, Category = "Simple Steam Input|Feedback")
	bool HasAdaptiveTriggers(int32 GamepadIndex) const;

	/**
	 * DualSense adaptive trigger effects on both triggers. No-op for other controllers. The effect stays until replaced.
	 * For per-player effects with durations and automatic resets, use the Steam Input Feedback functions instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input|Feedback")
	void SetDualSenseTriggerEffect(int32 GamepadIndex, const FSteamDualSenseTriggerEffect& LeftTrigger, const FSteamDualSenseTriggerEffect& RightTrigger);

	/** Like SetDualSenseTriggerEffect, but only the chosen trigger(s) change; the other keeps its effect. */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input|Feedback")
	void SetDualSenseTriggerEffectOn(int32 GamepadIndex, ESteamDualSenseTrigger Trigger, const FSteamDualSenseTriggerEffect& Effect);

	/** C++: one call for both triggers; a null trigger keeps its current effect. */
	void SetDualSenseTriggerEffects(int32 GamepadIndex, const FSteamDualSenseTriggerEffect* LeftTrigger, const FSteamDualSenseTriggerEffect* RightTrigger);

	UPROPERTY(BlueprintAssignable, Category = "Simple Steam Input")
	FOnSteamInputControllerConnected OnControllerConnected;

	/** ControllerType/GamepadIndex are the last known values (the handle is already gone in Steam). */
	UPROPERTY(BlueprintAssignable, Category = "Simple Steam Input")
	FOnSteamInputControllerDisconnected OnControllerDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "Simple Steam Input")
	FOnSteamInputOverlayActivated OnOverlayActivated;

	/** Native (non-dynamic) variants for C++ listeners. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOverlayActivatedNative, bool);
	FOnOverlayActivatedNative OnOverlayActivatedNative;
	DECLARE_MULTICAST_DELEGATE(FOnControllersChangedNative);
	FOnControllersChangedNative OnControllersChangedNative;

private:
	friend class FSteamInputCallbacks;

	bool Tick(float DeltaTime);
	void HandleDeviceConnected(uint64 Handle);
	void HandleDeviceDisconnected(uint64 Handle);
	void HandleOverlay(bool bActive);
	FSteamControllerInfo MakeInfo(uint64 Handle) const;
	uint64 HandleForGamepadIndex(int32 GamepadIndex) const;

	TSharedPtr<class FSteamClientInstanceHandler> SteamClient;
	/** TPimplPtr: deletable where FSteamInputCallbacks is incomplete (UHT-generated code destroys it). */
	TPimplPtr<class FSteamInputCallbacks> Callbacks;
	FTSTicker::FDelegateHandle TickHandle;

	/** Last known info per handle, so disconnects can report what went away. */
	TMap<uint64, FSteamControllerInfo> KnownControllers;

	bool bSteamInputReady = false;
	bool bOverlayActive = false;
};

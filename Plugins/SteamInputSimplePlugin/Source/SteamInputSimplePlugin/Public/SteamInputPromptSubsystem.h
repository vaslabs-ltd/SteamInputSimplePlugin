// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "SteamInputPromptTypes.h"
#include "SteamInputPromptSubsystem.generated.h"

class APlayerController;
class ULocalPlayer;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSteamInputControllerFamiliesChanged);

/**
 * Which controller family each local player holds (from Steam Input, matching the player's ControllerId to Steam's
 * emulated gamepad index), and the rich-text prompts and images that go with it. Without Steam everyone is Xbox.
 *
 * Prompts: templates use {A} {B} {X} {Y} {LB} {RB} {LT} {RT} {Start} {Back} {DPadLR} {DPadUD} {DPad} {DPadUp}
 * {DPadDown} {DPadLeft} {DPadRight} {LS} {RS} {L3} {R3} (Xbox names), each replaced by <Style>glyph</>. The rich text
 * block needs a style set with the glyph rows (the plugin's DT_SteamInputGlyphs, or a composite table that includes it).
 *
 * Shared screens (no owning player) follow the first local player.
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputPromptSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static USteamInputPromptSubsystem* Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Family of the pad that UE sees as ControllerId (Steam's emulated gamepad index). */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts")
	ESteamInputGlyphFamily GetFamilyForControllerId(int32 ControllerId) const;

	ESteamInputGlyphFamily GetFamilyForLocalPlayer(const ULocalPlayer* LocalPlayer) const;

	/** Owner's family, or the first local player's when Owner is null (shared screens). */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts")
	ESteamInputGlyphFamily GetPromptFamily(const APlayerController* Owner) const;

	/**
	 * Replaces the button tokens with rich-text glyphs for Family. Other {0}/{Name} arguments are left alone: fill them
	 * in with FText::Format *after* this (FText::Format with ordered arguments would turn {A} into "0").
	 * bSwapFaceButtons swaps A/B and X/Y (Switch pads that Steam maps by position).
	 */
	static FText FormatPrompt(const FText& Template, ESteamInputGlyphFamily Family, bool bSwapFaceButtons = false);

	/** FormatPrompt for Owner's controller (first local player's when Owner is null). */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts")
	FText FormatPromptFor(const FText& Template, const APlayerController* Owner) const;

	/** The family's version of an image with drawn-in buttons (Project Settings > Steam Input Prompts > Images). */
	static UTexture2D* GetTextureForFamily(UTexture2D* XboxTexture, ESteamInputGlyphFamily Family);

	/** Fires (once, shortly after) when pads connect or disconnect, so prompts and images can be rebuilt. */
	UPROPERTY(BlueprintAssignable, Category = "Steam Input|Prompts")
	FOnSteamInputControllerFamiliesChanged OnControllerFamiliesChanged;

	FSimpleMulticastDelegate OnControllerFamiliesChangedNative;

private:
	const ULocalPlayer* GetPromptPlayer(const APlayerController* Owner) const;
	bool ShouldSwapFaceButtons(int32 ControllerId, ESteamInputGlyphFamily Family) const;
	void HandleControllersChanged();
	void HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId);
	void LogFamilies() const;

	FDelegateHandle SteamControllersHandle;
	FDelegateHandle DeviceConnectionHandle;
	FTSTicker::FDelegateHandle DeferredBroadcastHandle;
};

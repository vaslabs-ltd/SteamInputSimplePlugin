// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamInputPromptTypes.h"
#include "SteamInputPromptLibrary.generated.h"

class APlayerController;
class UTexture2D;

/** Blueprint helpers for controller-specific prompts and images. */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputPromptLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Rich text with {A} {B} {X} {Y} {LB} {RB} {LT} {RT} {Start} {Back} {DPadLR} ... replaced by the glyphs of the
	 * owning player's controller (first local player's when Owning Player is empty). E.g. "Press {A} when Ready".
	 */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts", meta = (WorldContext = "WorldContext", AdvancedDisplay = "OwningPlayer"))
	static FText FormatControllerPrompt(const UObject* WorldContext, FText Template, APlayerController* OwningPlayer = nullptr);

	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts")
	static FText FormatControllerPromptForFamily(FText Template, ESteamInputGlyphFamily Family);

	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts", meta = (WorldContext = "WorldContext", AdvancedDisplay = "OwningPlayer"))
	static ESteamInputGlyphFamily GetControllerFamily(const UObject* WorldContext, APlayerController* OwningPlayer = nullptr);

	/** XboxTexture, or its version for the player's family (Project Settings > Steam Input Prompts > Images). */
	UFUNCTION(BlueprintPure, Category = "Steam Input|Prompts", meta = (WorldContext = "WorldContext", AdvancedDisplay = "OwningPlayer"))
	static UTexture2D* GetControllerTexture(const UObject* WorldContext, UTexture2D* XboxTexture, APlayerController* OwningPlayer = nullptr);
};

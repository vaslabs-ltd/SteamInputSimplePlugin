// Copyright Vaslabs LTD 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamInputTypes.h"
#include "SteamInputSimpleBlueprintLibrary.generated.h"


/* Delegate type to get notified on overlay events (activated/deactivated)*/
UDELEGATE(BlueprintCallable)
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSteamOverlayActivatedDelegate, bool, bActive);

/**
 * Backwards-compatible static API. New code should use USteamInputSubsystem (Get Engine Subsystem),
 * which also exposes connect/disconnect events and controller feedback.
 */
UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputSimpleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Function to get connected Steam controllers */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static TArray<FSteamControllerInfo> GetConnectedControllers();

	/* Function to get notified via a delegate when steam overlay is activated or deactivated. Replaces the previous binding. */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static void OnSteamOverlayStateChanged(const FOnSteamOverlayActivatedDelegate OnOverlayActivatedDelegate);

	/** Callbacks are now pumped automatically every frame; kept for compatibility. */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input", meta = (DeprecatedFunction, DeprecationMessage = "Steam callbacks are pumped automatically by USteamInputSubsystem."))
	static void RunSteamCallbacks();
};

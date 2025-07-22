// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamInputBlueprintLibrary.generated.h"

USTRUCT(BlueprintType)
struct FSteamControllerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ControllerHandle;

	UPROPERTY(BlueprintReadOnly)
	FString ControllerName;
};

UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns a list of connected Steam controllers
	UFUNCTION(BlueprintCallable, Category = "Steam Input")
	static TArray<FSteamControllerInfo> GetConnectedControllers();

	// Returns the glyph for a given action and controller
	UFUNCTION(BlueprintCallable, Category = "Steam Input")
	static FString GetGlyphForAction(int32 ControllerHandle, FString ActionName);
};

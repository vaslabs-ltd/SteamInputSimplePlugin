// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamInputBlueprintLibrary.generated.h"

UENUM(BlueprintType)
enum class ESteamControllerType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	SteamController UMETA(DisplayName = "Steam Controller"),
	Xbox360 UMETA(DisplayName = "Xbox 360 Controller"),
	XboxOne UMETA(DisplayName = "Xbox One Controller"),
	GenericGamepad UMETA(DisplayName = "Generic Gamepad"),
	PS4 UMETA(DisplayName = "PS4 Controller"),
	PS5 UMETA(DisplayName = "PS5 Controller"),
	SwitchPro UMETA(DisplayName = "Switch Pro Controller")
};

USTRUCT(BlueprintType)
struct FSteamControllerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam Controller Info")
	int32 ControllerHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Controller Info")
	ESteamControllerType ControllerType;
};


UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns a list of connected Steam controllers
	UFUNCTION(BlueprintCallable, Category = "Steam Input")
	static TArray<FSteamControllerInfo> GetConnectedControllers();
};

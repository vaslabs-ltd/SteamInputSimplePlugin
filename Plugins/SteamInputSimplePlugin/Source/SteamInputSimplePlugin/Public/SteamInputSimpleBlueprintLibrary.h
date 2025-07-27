// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "steam/steam_api.h"
#include "steam/isteaminput.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamInputSimpleBlueprintLibrary.generated.h"


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

	UPROPERTY(BlueprintReadOnly, Category = "Simple Steam Input")
	int32 ControllerHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Simple Steam Input")
	ESteamControllerType ControllerType;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamControllerConnected, int32, ControllerHandle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamControllerDisconnected, int32, ControllerHandle);

UCLASS(BlueprintType)
class STEAMINPUTSIMPLEPLUGIN_API USteamInputEventManager : public UObject
{
	GENERATED_BODY()
public:
	/** Called when a Steam controller is connected */
	UPROPERTY(BlueprintAssignable, Category = "Simple Steam Input")
	FOnSteamControllerConnected OnSteamControllerConnected;

	/** Called when a Steam controller is disconnected */
	UPROPERTY(BlueprintAssignable, Category = "Simple Steam Input")
	FOnSteamControllerDisconnected OnSteamControllerDisconnected;

	// Manual callback variables for Steam Input events
	STEAM_CALLBACK_MANUAL(USteamInputEventManager, OnSteamDeviceConnected, SteamInputDeviceConnected_t, DeviceConnectedCallback);
	STEAM_CALLBACK_MANUAL(USteamInputEventManager, OnSteamDeviceDisconnected, SteamInputDeviceDisconnected_t, DeviceDisconnectedCallback);
};

UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputSimpleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Function to get connected Steam controllers */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static TArray<FSteamControllerInfo> GetConnectedControllers();

	/** Function to enable device callbacks and get the input event manager for binding to callbacks*/
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static USteamInputEventManager* EnableSteamDeviceCallbacks();
};
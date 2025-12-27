// Copyright Vaslabs LTD 2025 All Rights Reserved.

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


/* Delegate type to get notified on overlay events (activated/deactivated)*/
UDELEGATE(BlueprintCallable)
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSteamOverlayActivatedDelegate, bool, bActive);

USTRUCT(BlueprintType)
struct FSteamControllerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Simple Steam Input")
	ESteamControllerType ControllerType;

};

UCLASS()
class STEAMINPUTSIMPLEPLUGIN_API USteamInputSimpleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Function to get connected Steam controllers */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static TArray<FSteamControllerInfo> GetConnectedControllers();

	/* Function to get notified via a delegate when steam overlay is activated or deactivated */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static void OnSteamOverlayStateChanged(const FOnSteamOverlayActivatedDelegate OnOverlayActivatedDelegate);

	/** Function to pump Steam callbacks - call this regularly to process Steam events */
	UFUNCTION(BlueprintCallable, Category = "Simple Steam Input")
	static void RunSteamCallbacks();
};
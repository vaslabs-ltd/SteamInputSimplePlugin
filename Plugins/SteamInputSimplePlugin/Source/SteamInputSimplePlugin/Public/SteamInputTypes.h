// Copyright Vaslabs LTD 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SteamInputTypes.generated.h"

/** Physical controller type as reported by Steam Input. New values are appended to keep Blueprint values stable. */
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
	SwitchPro UMETA(DisplayName = "Switch Pro Controller"),
	PS3 UMETA(DisplayName = "PS3 Controller"),
	SteamDeck UMETA(DisplayName = "Steam Deck"),
	/** A pair of Joy-Cons used together (like a Pro Controller). */
	SwitchJoyCon UMETA(DisplayName = "Switch Joy-Con Pair"),
	MobileTouch UMETA(DisplayName = "Steam Link Touch Controller"),
	/** One Joy-Con on its own, held sideways. */
	SwitchJoyConSingle UMETA(DisplayName = "Switch Joy-Con (single)")
};

USTRUCT(BlueprintType)
struct FSteamControllerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Simple Steam Input")
	ESteamControllerType ControllerType = ESteamControllerType::Unknown;

	/** Full 64-bit Steam Input handle (InputHandle_t). */
	UPROPERTY(BlueprintReadOnly, Category = "Simple Steam Input")
	int64 Handle = 0;

	/**
	 * Index of the emulated gamepad (XInput slot on Windows) that Steam presents this controller as,
	 * or -1 if the controller is not exposed as an emulated gamepad.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Simple Steam Input")
	int32 GamepadIndex = -1;
};

UENUM(BlueprintType)
enum class ESteamDualSenseTriggerMode : uint8
{
	Off,
	/** Constant resistance from StartPosition onwards. */
	Feedback,
	/** Click-like resistance between StartPosition and EndPosition (like a trigger pull). */
	Weapon,
	/** Vibrates from StartPosition onwards. */
	Vibration
};

USTRUCT(BlueprintType)
struct FSteamDualSenseTriggerEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input")
	ESteamDualSenseTriggerMode Mode = ESteamDualSenseTriggerMode::Off;

	/** Feedback/Vibration: 0..9. Weapon: 2..7 (clamped). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 0, ClampMax = 9))
	int32 StartPosition = 0;

	/** Weapon only, StartPosition+1..8 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 1, ClampMax = 8))
	int32 EndPosition = 8;

	/** 0..8 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 0, ClampMax = 8))
	int32 Strength = 8;

	/** Vibration only, Hz 0..255 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 0, ClampMax = 255))
	int32 Frequency = 0;
};

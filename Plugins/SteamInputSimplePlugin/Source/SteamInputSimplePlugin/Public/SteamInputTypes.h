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
	/** Constant resistance from StartPosition onwards. Strength 8 from position 0 is the closest to a locked trigger. */
	Feedback,
	/** Click-like resistance between StartPosition and EndPosition (like a trigger pull). */
	Weapon,
	/** Vibrates from StartPosition onwards. */
	Vibration,
	/** Resistance per zone: ZoneStrengths[0..9], one per trigger position (0 = no resistance in that zone). */
	MultiplePositionFeedback,
	/** Resistance ramping from Strength at StartPosition to EndStrength at EndPosition. */
	SlopeFeedback,
	/** Vibration amplitude per zone: ZoneStrengths[0..9] at Frequency. */
	MultiplePositionVibration
};

/** Which adaptive trigger(s) an effect goes to. */
UENUM(BlueprintType)
enum class ESteamDualSenseTrigger : uint8
{
	/** L2 (Xbox LT). */
	Left,
	/** R2 (Xbox RT). */
	Right,
	Both
};

/** Ready-made trigger effects (see USteamInputFeedbackLibrary::MakeTriggerEffectFromPreset for the values). */
UENUM(BlueprintType)
enum class ESteamDualSenseTriggerPreset : uint8
{
	Off,
	/** Light constant resistance after the first quarter of the pull. */
	Light,
	/** Strong resistance over the whole pull. */
	Heavy,
	/** Maximum resistance from the very start: the trigger feels locked (it can still be forced). Use for cooldowns. */
	Locked,
	/** A gun-like click part way through the pull. */
	Weapon,
	/** Resistance that builds up as the trigger goes down (a bow or a charge). */
	Ramp,
	/** Buzzes from the start of the pull (a beam or a rapid-fire laser). */
	Vibration
};

USTRUCT(BlueprintType)
struct FSteamDualSenseTriggerEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input")
	ESteamDualSenseTriggerMode Mode = ESteamDualSenseTriggerMode::Off;

	/** Positions run 0 (released) .. 9 (fully pulled). Feedback/Vibration: 0..9. Weapon: 2..7. Slope: 0..8 (clamped). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 0, ClampMax = 9))
	int32 StartPosition = 0;

	/** Weapon: StartPosition+1..8. Slope: StartPosition+1..9. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 1, ClampMax = 9))
	int32 EndPosition = 8;

	/** 0..8 (resistance, or vibration amplitude). Slope: the strength at StartPosition, 1..8. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 0, ClampMax = 8))
	int32 Strength = 8;

	/** Vibration / MultiplePositionVibration only, Hz 0..255 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 0, ClampMax = 255))
	int32 Frequency = 0;

	/** SlopeFeedback only: the strength at EndPosition, 1..8. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input", meta = (ClampMin = 1, ClampMax = 8))
	int32 EndStrength = 8;

	/** MultiplePosition modes only: 0..8 per zone, zone 0 (released) to zone 9 (fully pulled). Missing zones are 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Steam Input")
	TArray<int32> ZoneStrengths;

	bool operator==(const FSteamDualSenseTriggerEffect& Other) const
	{
		return Mode == Other.Mode && StartPosition == Other.StartPosition && EndPosition == Other.EndPosition && Strength == Other.Strength
			&& Frequency == Other.Frequency && EndStrength == Other.EndStrength && ZoneStrengths == Other.ZoneStrengths;
	}
	bool operator!=(const FSteamDualSenseTriggerEffect& Other) const { return !(*this == Other); }
};

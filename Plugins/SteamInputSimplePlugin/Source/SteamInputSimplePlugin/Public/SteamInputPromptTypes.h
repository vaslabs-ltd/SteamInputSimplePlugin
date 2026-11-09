// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "SteamInputPromptTypes.generated.h"

/** Which button glyphs a player sees. Generic pads (and no Steam) use Xbox. */
UENUM(BlueprintType)
enum class ESteamInputGlyphFamily : uint8
{
	Xbox,
	PlayStation,
	Switch,
	SteamDeck,
	SteamController,
	/** A single Joy-Con held sideways: its four face buttons by position, SL/SR as bumpers. */
	SwitchJoyCon
};

/**
 * Buttons that can appear in prompts, named after the Xbox layout (A = bottom face button, Cross on PlayStation).
 * The name is the prompt token: {A}, {LB}, {DPadLR}, ...
 */
UENUM(BlueprintType)
enum class ESteamInputPromptButton : uint8
{
	A,
	B,
	X,
	Y,
	LB,
	RB,
	LT,
	RT,
	Start,
	Back,
	/** D-pad left / right as one glyph. */
	DPadLR,
	/** D-pad up / down as one glyph. */
	DPadUD,
	DPad,
	DPadUp,
	DPadDown,
	DPadLeft,
	DPadRight,
	/** Left / right stick (move). */
	LS,
	RS,
	/** Left / right stick click. */
	L3,
	R3
};

/** How Switch face buttons line up with the Xbox-named tokens. */
UENUM(BlueprintType)
enum class ESteamInputSwitchLayout : uint8
{
	/** Ask Steam per controller (follows the player's "Use Nintendo Button Layout" choice). */
	Auto,
	/** {A} shows Switch A: Steam maps by label. */
	Labels,
	/** {A} shows Switch B (the bottom button): Steam maps by position. */
	Positions
};

UENUM(BlueprintType)
enum class ESteamInputAutoPauseReason : uint8
{
	ControllerDisconnected,
	SteamOverlay
};

/** How one button is written in rich text: <Style>Text</>. */
USTRUCT(BlueprintType)
struct FSteamInputPromptGlyph
{
	GENERATED_BODY()

	/** The character(s) to draw: a glyph font codepoint (Kenney Input Prompts) or plain text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Input|Prompts")
	FString Text;

	/** Rich text style row. Empty: "<Style Row Prefix>.<Family>.<Button>", e.g. Glyph.PS.A. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Input|Prompts")
	FName Style;
};

USTRUCT(BlueprintType)
struct FSteamInputGlyphSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Input|Prompts")
	TMap<ESteamInputPromptButton, FSteamInputPromptGlyph> Glyphs;
};

/** Versions of one image (drawn-in buttons) per family; families without one use the original. */
USTRUCT(BlueprintType)
struct FSteamInputImageVariants
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam Input|Prompts")
	TMap<ESteamInputGlyphFamily, TSoftObjectPtr<UTexture2D>> Variants;
};

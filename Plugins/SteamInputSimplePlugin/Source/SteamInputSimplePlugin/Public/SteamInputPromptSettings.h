// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SteamInputPromptTypes.h"
#include "SteamInputPromptSettings.generated.h"

/**
 * Project Settings > Plugins > Steam Input Prompts: the glyph for each button per controller family, the rich text
 * style rows that draw them, and image variants. The defaults use the Kenney Input Prompts fonts and the rows of
 * the plugin's DT_SteamInputGlyphs (Glyph.<Family>.<Button>).
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Steam Input Prompts"))
class STEAMINPUTSIMPLEPLUGIN_API USteamInputPromptSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USteamInputPromptSettings();

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	static const USteamInputPromptSettings& Get() { return *GetDefault<USteamInputPromptSettings>(); }

	/** The glyph for Button, falling back to Xbox's when Family has none (e.g. Steam Controller R3); a single Joy-Con
	 * tries Switch's first. */
	const FSteamInputPromptGlyph* FindGlyph(ESteamInputGlyphFamily Family, ESteamInputPromptButton Button, ESteamInputGlyphFamily* OutFamily = nullptr) const;

	/** The style row for a glyph: its own Style, or "<Style Row Prefix>.<Family>.<Button>". */
	FName GetStyleRow(ESteamInputGlyphFamily Family, ESteamInputPromptButton Button, const FSteamInputPromptGlyph& Glyph) const;

	/** "Xbox", "PS", "Switch", "SteamDeck", "SteamController", "SwitchJoyCon": the family part of style row names. */
	static FString GetFamilyKey(ESteamInputGlyphFamily Family);

	/** "A", "LB", "DPadLR", ...: the token is {<this>}. */
	static FString GetButtonToken(ESteamInputPromptButton Button);

	/** Default style rows are <Prefix>.<Family>.<Button>. */
	UPROPERTY(Config, EditAnywhere, Category = "Glyphs")
	FString StyleRowPrefix = TEXT("Glyph");

	UPROPERTY(Config, EditAnywhere, Category = "Glyphs")
	TMap<ESteamInputGlyphFamily, FSteamInputGlyphSet> Families;

	UPROPERTY(Config, EditAnywhere, Category = "Glyphs")
	ESteamInputSwitchLayout SwitchLayout = ESteamInputSwitchLayout::Auto;

	/**
	 * Images with controller buttons drawn in (help pages, rules): the Xbox version -> versions for other families.
	 * Keep the variants in a cooked folder.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Images")
	TMap<TSoftObjectPtr<UTexture2D>, FSteamInputImageVariants> Images;
};

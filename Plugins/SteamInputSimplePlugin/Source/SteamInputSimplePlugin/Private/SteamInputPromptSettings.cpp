// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPromptSettings.h"

namespace
{
	using EButton = ESteamInputPromptButton;

	/** Kenney Input Prompts 1.5 codepoints (filled style), in ESteamInputPromptButton order. 0 = no glyph. */
	struct FFamilyCodepoints
	{
		ESteamInputGlyphFamily Family;
		uint32 Codepoints[21];
	};

	//                                  A       B       X       Y       LB      RB      LT      RT      Start   Back    DPadLR  DPadUD  DPad    Up      Down    Left    Right   LS      RS      L3      R3
	const FFamilyCodepoints KenneyGlyphDefaults[] = {
		{ ESteamInputGlyphFamily::Xbox,            { 0xE004, 0xE006, 0xE01E, 0xE020, 0xE043, 0xE049, 0xE047, 0xE04D, 0xE014, 0xE01C, 0xE026, 0xE037, 0xE022, 0xE035, 0xE024, 0xE028, 0xE02B, 0xE04F, 0xE057, 0xE053, 0xE05B } },
		// PS4 Options / Share for Start / Back (PS5: 0xE024 / 0xE01E).
		{ ESteamInputGlyphFamily::PlayStation,     { 0xE04B, 0xE041, 0xE051, 0xE053, 0xE078, 0xE080, 0xE07C, 0xE084, 0xE009, 0xE00B, 0xE059, 0xE062, 0xE055, 0xE060, 0xE057, 0xE05B, 0xE05E, 0xE064, 0xE06C, 0xE04D, 0xE04F } },
		// By label ({A} = Switch A); the Switch Layout setting swaps A/B and X/Y for positional mapping. Plus / Minus for Start / Back.
		{ ESteamInputGlyphFamily::Switch,          { 0xE004, 0xE006, 0xE018, 0xE01A, 0xE00A, 0xE010, 0xE01C, 0xE01E, 0xE00E, 0xE00C, 0xE035, 0xE03E, 0xE031, 0xE03C, 0xE033, 0xE037, 0xE03A, 0xE05A, 0xE062, 0xE05E, 0xE066 } },
		{ ESteamInputGlyphFamily::SteamDeck,       { 0xE001, 0xE003, 0xE01D, 0xE01F, 0xE007, 0xE013, 0xE009, 0xE015, 0xE00F, 0xE01B, 0xE025, 0xE02E, 0xE021, 0xE02C, 0xE023, 0xE027, 0xE02A, 0xE030, 0xE038, 0xE034, 0xE03C } },
		// The d-pad is the left trackpad and RS the right trackpad; no right stick click (falls back to Xbox).
		{ ESteamInputGlyphFamily::SteamController, { 0xE022, 0xE024, 0xE036, 0xE038, 0xE049, 0xE055, 0xE04D, 0xE059, 0xE034, 0xE026, 0xE03E, 0xE047, 0xE03A, 0xE045, 0xE03C, 0xE040, 0xE043, 0xE05B, 0xE04F, 0xE05E, 0 } },
		// Single Joy-Con held sideways (Switch font): face buttons by position (the "one dot lit" glyphs), SL / SR as
		// bumpers, +/- for Start / Back, its one stick. Buttons it lacks fall back to Switch's glyphs.
		{ ESteamInputGlyphFamily::SwitchJoyCon,    { 0xE022, 0xE029, 0xE026, 0xE02B, 0xE012, 0xE014, 0,      0,      0xE00E, 0xE00C, 0,      0,      0,      0,      0,      0,      0,      0xE05A, 0,      0xE05E, 0 } },
	};
	static_assert(UE_ARRAY_COUNT(KenneyGlyphDefaults[0].Codepoints) == static_cast<int32>(EButton::R3) + 1, "One codepoint per ESteamInputPromptButton");
}

USteamInputPromptSettings::USteamInputPromptSettings()
{
	for (const FFamilyCodepoints& Family : KenneyGlyphDefaults)
	{
		FSteamInputGlyphSet& Set = Families.Add(Family.Family);
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Family.Codepoints); ++Index)
		{
			if (Family.Codepoints[Index])
			{
				FSteamInputPromptGlyph Glyph;
				Glyph.Text.AppendChar(static_cast<TCHAR>(Family.Codepoints[Index]));
				Set.Glyphs.Add(static_cast<EButton>(Index), Glyph);
			}
		}
	}
}

const FSteamInputPromptGlyph* USteamInputPromptSettings::FindGlyph(ESteamInputGlyphFamily Family, ESteamInputPromptButton Button, ESteamInputGlyphFamily* OutFamily) const
{
	// A single Joy-Con falls back to the Switch glyphs before Xbox's.
	const ESteamInputGlyphFamily Parent = Family == ESteamInputGlyphFamily::SwitchJoyCon ? ESteamInputGlyphFamily::Switch : ESteamInputGlyphFamily::Xbox;
	for (ESteamInputGlyphFamily Candidate : { Family, Parent, ESteamInputGlyphFamily::Xbox })
	{
		const FSteamInputGlyphSet* Set = Families.Find(Candidate);
		if (const FSteamInputPromptGlyph* Glyph = Set ? Set->Glyphs.Find(Button) : nullptr)
		{
			if (OutFamily)
			{
				*OutFamily = Candidate;
			}
			return Glyph;
		}
	}
	return nullptr;
}

FName USteamInputPromptSettings::GetStyleRow(ESteamInputGlyphFamily Family, ESteamInputPromptButton Button, const FSteamInputPromptGlyph& Glyph) const
{
	if (!Glyph.Style.IsNone())
	{
		return Glyph.Style;
	}
	return FName(FString::Printf(TEXT("%s.%s.%s"), *StyleRowPrefix, *GetFamilyKey(Family), *GetButtonToken(Button)));
}

FString USteamInputPromptSettings::GetFamilyKey(ESteamInputGlyphFamily Family)
{
	switch (Family)
	{
	case ESteamInputGlyphFamily::PlayStation: return TEXT("PS");
	case ESteamInputGlyphFamily::Switch: return TEXT("Switch");
	case ESteamInputGlyphFamily::SteamDeck: return TEXT("SteamDeck");
	case ESteamInputGlyphFamily::SteamController: return TEXT("SteamController");
	case ESteamInputGlyphFamily::SwitchJoyCon: return TEXT("SwitchJoyCon");
	default: return TEXT("Xbox");
	}
}

FString USteamInputPromptSettings::GetButtonToken(ESteamInputPromptButton Button)
{
	return StaticEnum<ESteamInputPromptButton>()->GetNameStringByValue(static_cast<int64>(Button));
}

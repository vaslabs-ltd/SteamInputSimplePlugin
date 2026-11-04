// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPromptSettings.h"
#include "SteamInputPromptSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace SteamInputPromptTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter;

	FString Markup(ESteamInputGlyphFamily Family, ESteamInputPromptButton Button)
	{
		const USteamInputPromptSettings& Settings = USteamInputPromptSettings::Get();
		ESteamInputGlyphFamily GlyphFamily = Family;
		const FSteamInputPromptGlyph* Glyph = Settings.FindGlyph(Family, Button, &GlyphFamily);
		check(Glyph);
		return FString::Printf(TEXT("<%s>%s</>"), *Settings.GetStyleRow(GlyphFamily, Button, *Glyph).ToString(), *Glyph->Text);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSteamInputPromptFormatTest, "SteamInputSimplePlugin.Prompts.Format", SteamInputPromptTests::Flags)

bool FSteamInputPromptFormatTest::RunTest(const FString& Parameters)
{
	using namespace SteamInputPromptTests;
	const FText Template = FText::FromString(TEXT("Press {A} to go, {Start} to quit, {DPadLR} to pick"));
	for (ESteamInputGlyphFamily Family : { ESteamInputGlyphFamily::Xbox, ESteamInputGlyphFamily::PlayStation, ESteamInputGlyphFamily::Switch,
		ESteamInputGlyphFamily::SteamDeck, ESteamInputGlyphFamily::SteamController })
	{
		const FString Expected = FString::Printf(TEXT("Press %s to go, %s to quit, %s to pick"), *Markup(Family, ESteamInputPromptButton::A),
			*Markup(Family, ESteamInputPromptButton::Start), *Markup(Family, ESteamInputPromptButton::DPadLR));
		TestEqual(*FString::Printf(TEXT("Tokens for %s"), *UEnum::GetValueAsString(Family)),
			USteamInputPromptSubsystem::FormatPrompt(Template, Family).ToString(), Expected);
	}
	TestTrue(TEXT("Default rows are Glyph.<Family>.<Button>"), Markup(ESteamInputGlyphFamily::PlayStation, ESteamInputPromptButton::A).StartsWith(TEXT("<Glyph.PS.A>")));
	TestNotEqual(TEXT("Families differ for A"), Markup(ESteamInputGlyphFamily::Xbox, ESteamInputPromptButton::A), Markup(ESteamInputGlyphFamily::PlayStation, ESteamInputPromptButton::A));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSteamInputPromptEdgeCasesTest, "SteamInputSimplePlugin.Prompts.EdgeCases", SteamInputPromptTests::Flags)

bool FSteamInputPromptEdgeCasesTest::RunTest(const FString& Parameters)
{
	using namespace SteamInputPromptTests;
	const ESteamInputGlyphFamily PS = ESteamInputGlyphFamily::PlayStation;

	// Glyphs first, ordered arguments after: {0} must survive FormatPrompt.
	const FText Scout = FText::Format(USteamInputPromptSubsystem::FormatPrompt(FText::FromString(TEXT(" {Y} Scout ({0} left)")), PS), 2);
	const FText Full = FText::Format(USteamInputPromptSubsystem::FormatPrompt(FText::FromString(TEXT("Press {A} to Continue{0}")), PS), Scout);
	TestEqual(TEXT("Nested prompt"), Full.ToString(), FString::Printf(TEXT("Press %s to Continue %s Scout (2 left)"),
		*Markup(PS, ESteamInputPromptButton::A), *Markup(PS, ESteamInputPromptButton::Y)));

	const FText Plain = FText::FromString(TEXT("No buttons here"));
	TestEqual(TEXT("Plain text untouched"), USteamInputPromptSubsystem::FormatPrompt(Plain, ESteamInputGlyphFamily::Xbox).ToString(), Plain.ToString());

	// {DPad} must not eat the start of {DPadLR} and vice versa.
	TestEqual(TEXT("Similar tokens"), USteamInputPromptSubsystem::FormatPrompt(FText::FromString(TEXT("{DPad}{DPadLR}")), PS).ToString(),
		Markup(PS, ESteamInputPromptButton::DPad) + Markup(PS, ESteamInputPromptButton::DPadLR));

	// Positional Switch mapping: the bottom button ({A}) is labelled B.
	TestEqual(TEXT("Switch by position"), USteamInputPromptSubsystem::FormatPrompt(FText::FromString(TEXT("{A}{Y}")), ESteamInputGlyphFamily::Switch, true).ToString(),
		Markup(ESteamInputGlyphFamily::Switch, ESteamInputPromptButton::B) + Markup(ESteamInputGlyphFamily::Switch, ESteamInputPromptButton::X));

	// Steam Controller has no right stick click: Xbox's glyph and row.
	TestEqual(TEXT("Fallback to Xbox"), USteamInputPromptSubsystem::FormatPrompt(FText::FromString(TEXT("{R3}")), ESteamInputGlyphFamily::SteamController).ToString(),
		Markup(ESteamInputGlyphFamily::Xbox, ESteamInputPromptButton::R3));
	return true;
}

#endif

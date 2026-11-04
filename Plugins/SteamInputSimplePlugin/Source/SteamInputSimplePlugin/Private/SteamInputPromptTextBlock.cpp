// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPromptTextBlock.h"
#include "SteamInputPromptSubsystem.h"
#include "GameFramework/PlayerController.h"

#define LOCTEXT_NAMESPACE "SteamInputPromptTextBlock"

FText USteamInputPromptTextBlock::FormatTemplate() const
{
	const APlayerController* Owner = PromptPlayer.Get();
	if (!Owner && bUseOwningPlayer)
	{
		Owner = GetOwningPlayer();
	}
	const USteamInputPromptSubsystem* Prompts = USteamInputPromptSubsystem::Get(this);
	return Prompts ? Prompts->FormatPromptFor(PromptTemplate, Owner) : USteamInputPromptSubsystem::FormatPrompt(PromptTemplate, ESteamInputGlyphFamily::Xbox);
}

void USteamInputPromptTextBlock::SynchronizeProperties()
{
	// Before Super, which pushes Text to the Slate widget. Also shows the Xbox glyphs in the designer.
	if (!PromptTemplate.IsEmpty())
	{
		SetText(FormatTemplate());
	}
	Super::SynchronizeProperties();
}

void USteamInputPromptTextBlock::RefreshPrompt()
{
	if (!PromptTemplate.IsEmpty())
	{
		SetText(FormatTemplate());
	}
}

void USteamInputPromptTextBlock::SetPromptTemplate(const FText& InTemplate)
{
	PromptTemplate = InTemplate;
	RefreshPrompt();
}

void USteamInputPromptTextBlock::SetPromptPlayer(APlayerController* Player)
{
	PromptPlayer = Player;
	RefreshPrompt();
}

TSharedRef<SWidget> USteamInputPromptTextBlock::RebuildWidget()
{
	WatchControllers();
	return Super::RebuildWidget();
}

void USteamInputPromptTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	UnwatchControllers();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void USteamInputPromptTextBlock::WatchControllers()
{
	USteamInputPromptSubsystem* Prompts = USteamInputPromptSubsystem::Get(this);
	if (!Prompts || WatchedPrompts.Get() == Prompts)
	{
		return;
	}
	UnwatchControllers();
	WatchedPrompts = Prompts;
	ControllersChangedHandle = Prompts->OnControllerFamiliesChangedNative.AddUObject(this, &USteamInputPromptTextBlock::RefreshPrompt);
}

void USteamInputPromptTextBlock::UnwatchControllers()
{
	if (USteamInputPromptSubsystem* Prompts = WatchedPrompts.Get())
	{
		Prompts->OnControllerFamiliesChangedNative.Remove(ControllersChangedHandle);
	}
	WatchedPrompts.Reset();
	ControllersChangedHandle.Reset();
}

#if WITH_EDITOR
const FText USteamInputPromptTextBlock::GetPaletteCategory()
{
	return LOCTEXT("PaletteCategory", "Steam Input");
}
#endif

#undef LOCTEXT_NAMESPACE

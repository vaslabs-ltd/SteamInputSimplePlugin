// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPromptLibrary.h"
#include "SteamInputPromptSubsystem.h"

ESteamInputGlyphFamily USteamInputPromptLibrary::GetControllerFamily(const UObject* WorldContext, APlayerController* OwningPlayer)
{
	const USteamInputPromptSubsystem* Prompts = USteamInputPromptSubsystem::Get(WorldContext);
	return Prompts ? Prompts->GetPromptFamily(OwningPlayer) : ESteamInputGlyphFamily::Xbox;
}

FText USteamInputPromptLibrary::FormatControllerPrompt(const UObject* WorldContext, FText Template, APlayerController* OwningPlayer)
{
	const USteamInputPromptSubsystem* Prompts = USteamInputPromptSubsystem::Get(WorldContext);
	return Prompts ? Prompts->FormatPromptFor(Template, OwningPlayer) : USteamInputPromptSubsystem::FormatPrompt(Template, ESteamInputGlyphFamily::Xbox);
}

FText USteamInputPromptLibrary::FormatControllerPromptForFamily(FText Template, ESteamInputGlyphFamily Family)
{
	return USteamInputPromptSubsystem::FormatPrompt(Template, Family);
}

UTexture2D* USteamInputPromptLibrary::GetControllerTexture(const UObject* WorldContext, UTexture2D* XboxTexture, APlayerController* OwningPlayer)
{
	return USteamInputPromptSubsystem::GetTextureForFamily(XboxTexture, GetControllerFamily(WorldContext, OwningPlayer));
}

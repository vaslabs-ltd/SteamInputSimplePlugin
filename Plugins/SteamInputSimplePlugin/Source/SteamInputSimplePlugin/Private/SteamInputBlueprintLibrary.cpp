
// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteamInputBlueprintLibrary.h"
#include "SteamSharedModule.h"

TArray<FSteamControllerInfo> USteamInputBlueprintLibrary::GetConnectedControllers()
{
// ...existing code...
TArray<FSteamControllerInfo> Controllers;
if (FSteamSharedModule::IsAvailable())
{
	TSharedPtr<FSteamClientInstanceHandler> SteamClientHandler = FSteamSharedModule::Get().ObtainSteamClientInstanceHandle();
	if (SteamClientHandler.IsValid() && SteamClientHandler->IsInitialized())
	{
		// Access Steam Input API via handler (example, may need to cast or access underlying API)
		// This is a placeholder; you may need to expose SteamInput() from the handler or use a helper
		// Example:
		// ISteamInput* SteamInputAPI = SteamClientHandler->GetSteamInput();
		// if (SteamInputAPI) { ... }
		// For now, just stub:
		// TODO: Replace with actual Steam Input API access via SteamClientHandler
	}
}
return Controllers;
}

FString USteamInputBlueprintLibrary::GetGlyphForAction(int32 ControllerHandle, FString ActionName)
{
	// TODO: Implement Steam Input API integration
	return FString();
}

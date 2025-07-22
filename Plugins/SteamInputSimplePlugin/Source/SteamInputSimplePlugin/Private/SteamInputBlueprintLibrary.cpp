
// Copyright Epic Games, Inc. All Rights Reserved.


#include "SteamInputBlueprintLibrary.h"
#include "SteamSharedModule.h"
#include "steam/steam_api.h"
#include "steam/isteaminput.h"

TArray<FSteamControllerInfo> USteamInputBlueprintLibrary::GetConnectedControllers()
{
TArray<FSteamControllerInfo> Controllers;
if (FSteamSharedModule::IsAvailable())
{
	TSharedPtr<FSteamClientInstanceHandler> SteamClientHandler = FSteamSharedModule::Get().ObtainSteamClientInstanceHandle();
	if (SteamClientHandler.IsValid() && SteamClientHandler->IsInitialized())
	{
		ISteamInput* SteamInputAPI = SteamInput();
		if (SteamInputAPI)
		{
			SteamInputAPI->RunFrame();

			InputHandle_t Handles[STEAM_INPUT_MAX_COUNT];
			int Count = SteamInputAPI->GetConnectedControllers(Handles);

			for (int i = 0; i < Count; ++i)
			{
				FSteamControllerInfo Info;
				Info.ControllerHandle = (int32)Handles[i];

				ESteamInputType Type = SteamInputAPI->GetInputTypeForHandle(Handles[i]);
				FString TypeName;
				switch (Type)
				{
					case k_ESteamInputType_SteamController: TypeName = TEXT("Steam Controller"); break;
					case k_ESteamInputType_XBox360Controller: TypeName = TEXT("Xbox 360 Controller"); break;
					case k_ESteamInputType_XBoxOneController: TypeName = TEXT("Xbox One Controller"); break;
					case k_ESteamInputType_GenericGamepad: TypeName = TEXT("Generic Gamepad"); break;
					case k_ESteamInputType_PS4Controller: TypeName = TEXT("PS4 Controller"); break;
					case k_ESteamInputType_PS5Controller: TypeName = TEXT("PS5 Controller"); break;
					case k_ESteamInputType_SwitchProController: TypeName = TEXT("Switch Pro Controller"); break;
					case k_ESteamInputType_Unknown: default: TypeName = TEXT("Unknown Controller"); break;
				}
				Info.ControllerName = TypeName;
				Controllers.Add(Info);
			}
		}
	}
}
return Controllers;
}

FString USteamInputBlueprintLibrary::GetGlyphForAction(int32 ControllerHandle, FString ActionName)
{
	// TODO: Implement Steam Input API integration
	return FString();
}

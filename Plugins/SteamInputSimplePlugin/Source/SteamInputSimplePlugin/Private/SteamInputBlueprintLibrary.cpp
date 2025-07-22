
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
                SteamInputAPI->Init(true);
                SteamInputAPI->RunFrame();


                InputHandle_t Handles[STEAM_INPUT_MAX_COUNT];
                int Count = SteamInputAPI->GetConnectedControllers(Handles);

                for (int i = 0; i < Count; ++i)
                {
                    FSteamControllerInfo Info;
                    Info.ControllerHandle = (int32)Handles[i];

                    ESteamInputType Type = SteamInputAPI->GetInputTypeForHandle(Handles[i]);
                    switch (Type)
                    {
                        case k_ESteamInputType_SteamController: Info.ControllerType = ESteamControllerType::SteamController; break;
                        case k_ESteamInputType_XBox360Controller: Info.ControllerType = ESteamControllerType::Xbox360; break;
                        case k_ESteamInputType_XBoxOneController: Info.ControllerType = ESteamControllerType::XboxOne; break;
                        case k_ESteamInputType_GenericGamepad: Info.ControllerType = ESteamControllerType::GenericGamepad; break;
                        case k_ESteamInputType_PS4Controller: Info.ControllerType = ESteamControllerType::PS4; break;
                        case k_ESteamInputType_PS5Controller: Info.ControllerType = ESteamControllerType::PS5; break;
                        case k_ESteamInputType_SwitchProController: Info.ControllerType = ESteamControllerType::SwitchPro; break;
                        case k_ESteamInputType_Unknown: default: Info.ControllerType = ESteamControllerType::Unknown; break;
                    }
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

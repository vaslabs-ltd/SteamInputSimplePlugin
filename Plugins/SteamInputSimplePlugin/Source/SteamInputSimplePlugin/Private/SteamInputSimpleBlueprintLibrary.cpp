// Copyright Vaslabs LTD 2025 All Rights Reserved.

#include "SteamInputSimpleBlueprintLibrary.h"
#include "SteamSharedModule.h"
#include "steam/steam_api.h"
#include "steam/isteaminput.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamInputSimplePlugin, Log, All);

TArray<FSteamControllerInfo> USteamInputSimpleBlueprintLibrary::GetConnectedControllers()
{
    UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("Steam Input API is initialized."));

    TArray<FSteamControllerInfo> Controllers;
    if (FSteamSharedModule::IsAvailable())
    {
        TSharedPtr<FSteamClientInstanceHandler> SteamClientHandler = 
            FSteamSharedModule::Get().ObtainSteamClientInstanceHandle();
        if (SteamClientHandler.IsValid() && SteamClientHandler->IsInitialized())
        {
            ISteamInput* SteamInputAPI = SteamInput();

            if (SteamInputAPI)
            {
                SteamInputAPI->Init(true);
                SteamInputAPI->RunFrame();

                InputHandle_t Handles[STEAM_INPUT_MAX_COUNT];
                int Count = SteamInputAPI->GetConnectedControllers(Handles);
                Controllers.Reserve(Count);

                for (int i = 0; i < Count; ++i)
                {
                    FSteamControllerInfo Info;

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
                SteamInputAPI->Shutdown();
            }
        } else {
            // Print out an error message if Steam Input API is not available
            UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("Steam Input API is not available or Steam Client is not initialized."));
        }
    } else {
        // Print out an error message if Steam Shared Module is not available
        UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("Steam Shared Module is not available."));
    }
    return Controllers;
}

void USteamInputSimpleBlueprintLibrary::RunSteamCallbacks()
{
    SteamAPI_RunCallbacks();
}

// Static storage for the overlay delegate
static FOnSteamOverlayActivatedDelegate StoredOverlayDelegate;

// Custom callback handler class for Steam overlay activation
class COverlayActivatedCallbackHandler : public CCallbackImpl<sizeof(GameOverlayActivated_t)>
{
public:
    COverlayActivatedCallbackHandler()
    {
        m_iCallback = GameOverlayActivated_t::k_iCallback;
        SteamAPI_RegisterCallback(this, GameOverlayActivated_t::k_iCallback);
    }
    
    ~COverlayActivatedCallbackHandler()
    {
        SteamAPI_UnregisterCallback(this);
    }

private:
    virtual void Run(void* pvParam) override
    {
        GameOverlayActivated_t* pParam = (GameOverlayActivated_t*)pvParam;
        if (StoredOverlayDelegate.IsBound())
        {
            StoredOverlayDelegate.ExecuteIfBound(pParam->m_bActive != 0);
        }
    }
};

void USteamInputSimpleBlueprintLibrary::OnSteamOverlayStateChanged(const FOnSteamOverlayActivatedDelegate OnOverlayActivatedDelegate)
{
    // Store the delegate for the callback
    StoredOverlayDelegate = OnOverlayActivatedDelegate;
    
    if (FSteamSharedModule::IsAvailable())
    {
        TSharedPtr<FSteamClientInstanceHandler> SteamClientHandler = 
            FSteamSharedModule::Get().ObtainSteamClientInstanceHandle();
        if (SteamClientHandler.IsValid() && SteamClientHandler->IsInitialized())
        {
            ISteamUtils* SteamUtilsAPI = SteamUtils();

            if (SteamUtilsAPI)
            {
                // Create static callback handler that will remain alive
                static COverlayActivatedCallbackHandler* OverlayCallbackHandler = nullptr;
                if (!OverlayCallbackHandler)
                {
                    OverlayCallbackHandler = new COverlayActivatedCallbackHandler();
                }
            }
        } else {
            // Print out an error message if Steam Utils API is not available
            UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("Steam Utils API is not available or Steam Client is not initialized."));
        }
    } else {
        // Print out an error message if Steam Shared Module is not available
        UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("Steam Shared Module is not available."));
    }
}
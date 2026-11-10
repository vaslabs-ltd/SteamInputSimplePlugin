// Copyright Vaslabs LTD 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SteamInputTypes.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
#include "steam/isteaminput.h"
#include "steam/isteamdualsense.h"
THIRD_PARTY_INCLUDES_END

DECLARE_LOG_CATEGORY_EXTERN(LogSteamInputSimplePlugin, Log, All);

namespace SteamInputSimple
{
	inline ESteamControllerType ToControllerType(ESteamInputType Type)
	{
		switch (Type)
		{
		case k_ESteamInputType_SteamController: return ESteamControllerType::SteamController;
		case k_ESteamInputType_XBox360Controller: return ESteamControllerType::Xbox360;
		case k_ESteamInputType_XBoxOneController: return ESteamControllerType::XboxOne;
		case k_ESteamInputType_GenericGamepad: return ESteamControllerType::GenericGamepad;
		case k_ESteamInputType_PS4Controller: return ESteamControllerType::PS4;
		case k_ESteamInputType_PS5Controller: return ESteamControllerType::PS5;
		case k_ESteamInputType_PS3Controller: return ESteamControllerType::PS3;
		case k_ESteamInputType_SwitchProController: return ESteamControllerType::SwitchPro;
		case k_ESteamInputType_SwitchJoyConPair: return ESteamControllerType::SwitchJoyCon;
		case k_ESteamInputType_SwitchJoyConSingle: return ESteamControllerType::SwitchJoyConSingle;
		case k_ESteamInputType_SteamDeckController: return ESteamControllerType::SteamDeck;
		case k_ESteamInputType_MobileTouch: return ESteamControllerType::MobileTouch;
		default: return ESteamControllerType::Unknown;
		}
	}

	/** Steam's command for one trigger, with every value clamped to the range its mode accepts. */
	void FillTriggerCommand(const FSteamDualSenseTriggerEffect& Effect, ScePadTriggerEffectCommand& Out);
}

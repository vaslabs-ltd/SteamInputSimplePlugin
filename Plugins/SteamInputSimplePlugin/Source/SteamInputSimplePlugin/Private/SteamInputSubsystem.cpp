// Copyright Vaslabs LTD 2025 All Rights Reserved.

#include "SteamInputSubsystem.h"
#include "SteamInputSimpleInternal.h"
#include "SteamSharedModule.h"
#include "Engine/Engine.h"

#if PLATFORM_LINUX
#include <stdlib.h>

namespace SteamInputSimple
{
	/**
	 * Steam Input only feeds a game's controller config while Steam sees the game window focused, and Steam tracks
	 * focus through X11. Under a Wayland session SDL3 opens a native Wayland window, Steam never activates the game's
	 * config and every pad goes dead (Proton is fine: Wine always uses X11). So, when Steam launched us, ask SDL for
	 * X11 (XWayland). This runs at program start, before the engine initialises SDL, in monolithic (packaged) builds;
	 * in the modular editor the module loads too late and it does nothing. An explicit SDL_VIDEODRIVER /
	 * SDL_VIDEO_DRIVER always wins.
	 */
	struct FPreferX11UnderSteam
	{
		bool bApplied = false;

		FPreferX11UnderSteam()
		{
			const bool bLaunchedBySteam = getenv("SteamEnv") || getenv("SteamGameId") || getenv("SteamAppId");
			const bool bDriverChosen = getenv("SDL_VIDEODRIVER") || getenv("SDL_VIDEO_DRIVER");
			if (bLaunchedBySteam && !bDriverChosen && getenv("DISPLAY") && getenv("WAYLAND_DISPLAY"))
			{
				bApplied = setenv("SDL_VIDEO_DRIVER", "x11", 0) == 0;
			}
		}
	};
	static FPreferX11UnderSteam GPreferX11UnderSteam;
}
#endif

THIRD_PARTY_INCLUDES_START
#include "steam/isteamdualsense.h"
#include "steam/isteamutils.h"
THIRD_PARTY_INCLUDES_END

/** Owns the Steam callback registrations (kept out of the public header so it stays Steam-free). */
class FSteamInputCallbacks
{
public:
	explicit FSteamInputCallbacks(USteamInputSubsystem* InOwner) : Owner(InOwner)
	{
		ConnectedCallback.Register(this, &FSteamInputCallbacks::OnConnected);
		DisconnectedCallback.Register(this, &FSteamInputCallbacks::OnDisconnected);
		OverlayCallback.Register(this, &FSteamInputCallbacks::OnOverlay);
	}

	~FSteamInputCallbacks()
	{
		ConnectedCallback.Unregister();
		DisconnectedCallback.Unregister();
		OverlayCallback.Unregister();
	}

private:
	STEAM_CALLBACK_MANUAL(FSteamInputCallbacks, OnConnected, SteamInputDeviceConnected_t, ConnectedCallback);
	STEAM_CALLBACK_MANUAL(FSteamInputCallbacks, OnDisconnected, SteamInputDeviceDisconnected_t, DisconnectedCallback);
	STEAM_CALLBACK_MANUAL(FSteamInputCallbacks, OnOverlay, GameOverlayActivated_t, OverlayCallback);

	USteamInputSubsystem* Owner;
};

void FSteamInputCallbacks::OnConnected(SteamInputDeviceConnected_t* Param)
{
	Owner->HandleDeviceConnected(Param->m_ulConnectedDeviceHandle);
}

void FSteamInputCallbacks::OnDisconnected(SteamInputDeviceDisconnected_t* Param)
{
	Owner->HandleDeviceDisconnected(Param->m_ulDisconnectedDeviceHandle);
}

void FSteamInputCallbacks::OnOverlay(GameOverlayActivated_t* Param)
{
	Owner->HandleOverlay(Param->m_bActive != 0);
}

USteamInputSubsystem::USteamInputSubsystem() = default;
USteamInputSubsystem::~USteamInputSubsystem() = default;

USteamInputSubsystem* USteamInputSubsystem::Get()
{
	return GEngine ? GEngine->GetEngineSubsystem<USteamInputSubsystem>() : nullptr;
}

void USteamInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if PLATFORM_LINUX
	if (SteamInputSimple::GPreferX11UnderSteam.bApplied)
	{
		UE_LOG(LogSteamInputSimplePlugin, Log, TEXT("Launched by Steam under Wayland: asked SDL for X11 so Steam Input can see the game window focused."));
	}
#endif

	if (IsRunningCommandlet() || IsRunningDedicatedServer() || !FSteamSharedModule::IsAvailable())
	{
		return;
	}

	SteamClient = FSteamSharedModule::Get().ObtainSteamClientInstanceHandle();
	if (!SteamClient.IsValid() || !SteamClient->IsInitialized() || !SteamInput())
	{
		UE_LOG(LogSteamInputSimplePlugin, Log, TEXT("Steam is not running; Steam Input features are disabled."));
		SteamClient.Reset();
		return;
	}

	// RunFrame happens inside SteamAPI_RunCallbacks, which we pump every tick.
	if (!SteamInput()->Init(false))
	{
		UE_LOG(LogSteamInputSimplePlugin, Warning, TEXT("SteamInput()->Init failed; Steam Input features are disabled."));
		SteamClient.Reset();
		return;
	}

	Callbacks = MakePimpl<FSteamInputCallbacks>(this);
	// Steam sends a connected event for every controller that is already plugged in.
	SteamInput()->EnableDeviceCallbacks();
	bSteamInputReady = true;

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &USteamInputSubsystem::Tick));
	UE_LOG(LogSteamInputSimplePlugin, Log, TEXT("Steam Input initialised."));
}

void USteamInputSubsystem::Deinitialize()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	Callbacks.Reset();
	if (bSteamInputReady && SteamInput())
	{
		SteamInput()->Shutdown();
	}
	bSteamInputReady = false;
	KnownControllers.Reset();
	SteamClient.Reset();

	Super::Deinitialize();
}

bool USteamInputSubsystem::Tick(float DeltaTime)
{
	if (bSteamInputReady)
	{
		SteamAPI_RunCallbacks();
	}
	return true;
}

FSteamControllerInfo USteamInputSubsystem::MakeInfo(uint64 Handle) const
{
	FSteamControllerInfo Info;
	Info.Handle = static_cast<int64>(Handle);
	if (bSteamInputReady && SteamInput())
	{
		Info.ControllerType = SteamInputSimple::ToControllerType(SteamInput()->GetInputTypeForHandle(Handle));
		Info.GamepadIndex = SteamInput()->GetGamepadIndexForController(Handle);
	}
	return Info;
}

void USteamInputSubsystem::HandleDeviceConnected(uint64 Handle)
{
	const FSteamControllerInfo Info = MakeInfo(Handle);
	KnownControllers.Add(Handle, Info);
	UE_LOG(LogSteamInputSimplePlugin, Log, TEXT("Controller connected: handle %llu type %s gamepad index %d"),
		Handle, *UEnum::GetValueAsString(Info.ControllerType), Info.GamepadIndex);
	OnControllerConnected.Broadcast(Info);
	OnControllersChangedNative.Broadcast();
}

void USteamInputSubsystem::HandleDeviceDisconnected(uint64 Handle)
{
	FSteamControllerInfo Info;
	if (!KnownControllers.RemoveAndCopyValue(Handle, Info))
	{
		Info.Handle = static_cast<int64>(Handle);
	}
	UE_LOG(LogSteamInputSimplePlugin, Log, TEXT("Controller disconnected: handle %llu type %s gamepad index %d"),
		Handle, *UEnum::GetValueAsString(Info.ControllerType), Info.GamepadIndex);
	OnControllerDisconnected.Broadcast(Info);
	OnControllersChangedNative.Broadcast();
}

void USteamInputSubsystem::HandleOverlay(bool bActive)
{
	bOverlayActive = bActive;
	OnOverlayActivated.Broadcast(bActive);
	OnOverlayActivatedNative.Broadcast(bActive);
}

TArray<FSteamControllerInfo> USteamInputSubsystem::GetConnectedControllers() const
{
	TArray<FSteamControllerInfo> Controllers;
	if (!bSteamInputReady || !SteamInput())
	{
		return Controllers;
	}
	InputHandle_t Handles[STEAM_INPUT_MAX_COUNT];
	const int32 Count = SteamInput()->GetConnectedControllers(Handles);
	Controllers.Reserve(Count);
	for (int32 i = 0; i < Count; ++i)
	{
		Controllers.Add(MakeInfo(Handles[i]));
	}
	return Controllers;
}

uint64 USteamInputSubsystem::HandleForGamepadIndex(int32 GamepadIndex) const
{
	if (!bSteamInputReady || !SteamInput() || GamepadIndex < 0)
	{
		return 0;
	}
	return SteamInput()->GetControllerForGamepadIndex(GamepadIndex);
}

ESteamControllerType USteamInputSubsystem::GetControllerTypeForGamepadIndex(int32 GamepadIndex) const
{
	const uint64 Handle = HandleForGamepadIndex(GamepadIndex);
	return Handle ? SteamInputSimple::ToControllerType(SteamInput()->GetInputTypeForHandle(Handle)) : ESteamControllerType::Unknown;
}

bool USteamInputSubsystem::UsesNintendoButtonLabels(int32 GamepadIndex) const
{
	const uint64 Handle = HandleForGamepadIndex(GamepadIndex);
	if (!Handle)
	{
		return true;
	}
	// Pro controller / Joy-Con pair: Xbox A comes from the button labelled B when Steam maps by position.
	return SteamInput()->GetActionOriginFromXboxOrigin(Handle, k_EXboxOrigin_A) != k_EInputActionOrigin_Switch_B;
}

int32 USteamInputSubsystem::GetGamepadIndexForHandle(int64 Handle) const
{
	return (bSteamInputReady && SteamInput() && Handle) ? SteamInput()->GetGamepadIndexForController(static_cast<uint64>(Handle)) : -1;
}

void USteamInputSubsystem::TriggerVibration(int32 GamepadIndex, float LeftMotor, float RightMotor, float LeftTrigger, float RightTrigger)
{
	if (const uint64 Handle = HandleForGamepadIndex(GamepadIndex))
	{
		auto ToSpeed = [](float V) { return static_cast<unsigned short>(FMath::Clamp(V, 0.f, 1.f) * 65535.f); };
		SteamInput()->TriggerVibrationExtended(Handle, ToSpeed(LeftMotor), ToSpeed(RightMotor), ToSpeed(LeftTrigger), ToSpeed(RightTrigger));
	}
}

void USteamInputSubsystem::SetLightbarColor(int32 GamepadIndex, FLinearColor Color, bool bReset)
{
	if (const uint64 Handle = HandleForGamepadIndex(GamepadIndex))
	{
		const FColor C = Color.ToFColor(true);
		SteamInput()->SetLEDColor(Handle, C.R, C.G, C.B,
			bReset ? k_ESteamInputLEDFlag_RestoreUserDefault : k_ESteamInputLEDFlag_SetColor);
	}
}

namespace SteamInputSimple
{
	void FillTriggerCommand(const FSteamDualSenseTriggerEffect& Effect, ScePadTriggerEffectCommand& Out)
	{
		FMemory::Memzero(Out);
		auto Clamp8 = [](int32 V, int32 Min = 0) { return static_cast<uint8>(FMath::Clamp(V, Min, 8)); };
		const uint8 Strength = Clamp8(Effect.Strength);
		auto FillZones = [&Effect, &Clamp8](uint8* Zones)
		{
			for (int32 i = 0; i < SCE_PAD_TRIGGER_EFFECT_CONTROL_POINT_NUM; ++i)
			{
				Zones[i] = Effect.ZoneStrengths.IsValidIndex(i) ? Clamp8(Effect.ZoneStrengths[i]) : 0;
			}
		};
		switch (Effect.Mode)
		{
		case ESteamDualSenseTriggerMode::Feedback:
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_FEEDBACK;
			Out.commandData.feedbackParam.position = static_cast<uint8>(FMath::Clamp(Effect.StartPosition, 0, 9));
			Out.commandData.feedbackParam.strength = Strength;
			break;
		case ESteamDualSenseTriggerMode::Weapon:
		{
			const int32 Start = FMath::Clamp(Effect.StartPosition, 2, 7);
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_WEAPON;
			Out.commandData.weaponParam.startPosition = static_cast<uint8>(Start);
			Out.commandData.weaponParam.endPosition = static_cast<uint8>(FMath::Clamp(Effect.EndPosition, Start + 1, 8));
			Out.commandData.weaponParam.strength = Strength;
			break;
		}
		case ESteamDualSenseTriggerMode::Vibration:
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_VIBRATION;
			Out.commandData.vibrationParam.position = static_cast<uint8>(FMath::Clamp(Effect.StartPosition, 0, 9));
			Out.commandData.vibrationParam.amplitude = Strength;
			Out.commandData.vibrationParam.frequency = static_cast<uint8>(FMath::Clamp(Effect.Frequency, 0, 255));
			break;
		case ESteamDualSenseTriggerMode::MultiplePositionFeedback:
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_FEEDBACK;
			FillZones(Out.commandData.multiplePositionFeedbackParam.strength);
			break;
		case ESteamDualSenseTriggerMode::SlopeFeedback:
		{
			const int32 Start = FMath::Clamp(Effect.StartPosition, 0, 8);
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_SLOPE_FEEDBACK;
			Out.commandData.slopeFeedbackParam.startPosition = static_cast<uint8>(Start);
			Out.commandData.slopeFeedbackParam.endPosition = static_cast<uint8>(FMath::Clamp(Effect.EndPosition, Start + 1, 9));
			Out.commandData.slopeFeedbackParam.startStrength = Clamp8(Effect.Strength, 1);
			Out.commandData.slopeFeedbackParam.endStrength = Clamp8(Effect.EndStrength, 1);
			break;
		}
		case ESteamDualSenseTriggerMode::MultiplePositionVibration:
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_VIBRATION;
			Out.commandData.multiplePositionVibrationParam.frequency = static_cast<uint8>(FMath::Clamp(Effect.Frequency, 0, 255));
			FillZones(Out.commandData.multiplePositionVibrationParam.amplitude);
			break;
		case ESteamDualSenseTriggerMode::Off:
		default:
			Out.mode = SCE_PAD_TRIGGER_EFFECT_MODE_OFF;
			break;
		}
	}
}

bool USteamInputSubsystem::HasAdaptiveTriggers(int32 GamepadIndex) const
{
	return GetControllerTypeForGamepadIndex(GamepadIndex) == ESteamControllerType::PS5;
}

void USteamInputSubsystem::SetDualSenseTriggerEffect(int32 GamepadIndex, const FSteamDualSenseTriggerEffect& LeftTrigger, const FSteamDualSenseTriggerEffect& RightTrigger)
{
	SetDualSenseTriggerEffects(GamepadIndex, &LeftTrigger, &RightTrigger);
}

void USteamInputSubsystem::SetDualSenseTriggerEffectOn(int32 GamepadIndex, ESteamDualSenseTrigger Trigger, const FSteamDualSenseTriggerEffect& Effect)
{
	SetDualSenseTriggerEffects(GamepadIndex, Trigger != ESteamDualSenseTrigger::Right ? &Effect : nullptr,
		Trigger != ESteamDualSenseTrigger::Left ? &Effect : nullptr);
}

void USteamInputSubsystem::SetDualSenseTriggerEffects(int32 GamepadIndex, const FSteamDualSenseTriggerEffect* LeftTrigger, const FSteamDualSenseTriggerEffect* RightTrigger)
{
	const uint64 Handle = HandleForGamepadIndex(GamepadIndex);
	if (!Handle || (!LeftTrigger && !RightTrigger) || SteamInput()->GetInputTypeForHandle(Handle) != k_ESteamInputType_PS5Controller)
	{
		return;
	}
	// Only the masked triggers change; the other one keeps whatever effect it had.
	ScePadTriggerEffectParam Param;
	FMemory::Memzero(Param);
	if (LeftTrigger)
	{
		Param.triggerMask |= SCE_PAD_TRIGGER_EFFECT_TRIGGER_MASK_L2;
		SteamInputSimple::FillTriggerCommand(*LeftTrigger, Param.command[SCE_PAD_TRIGGER_EFFECT_PARAM_INDEX_FOR_L2]);
	}
	if (RightTrigger)
	{
		Param.triggerMask |= SCE_PAD_TRIGGER_EFFECT_TRIGGER_MASK_R2;
		SteamInputSimple::FillTriggerCommand(*RightTrigger, Param.command[SCE_PAD_TRIGGER_EFFECT_PARAM_INDEX_FOR_R2]);
	}
	SteamInput()->SetDualSenseTriggerEffect(Handle, &Param);
}

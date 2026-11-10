// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputFeedbackLibrary.h"
#include "SteamInputFeedbackSubsystem.h"
#include "SteamInputSubsystem.h"
#include "GameFramework/PlayerController.h"

void USteamInputFeedbackLibrary::SetTriggerEffect(APlayerController* Player, ESteamDualSenseTrigger Trigger, FSteamDualSenseTriggerEffect Effect, float Duration)
{
	if (USteamInputFeedbackSubsystem* Feedback = USteamInputFeedbackSubsystem::Get(Player))
	{
		Feedback->SetTriggerEffect(USteamInputFeedbackSubsystem::GetGamepadIndex(Player), Trigger, Effect, Duration);
	}
}

void USteamInputFeedbackLibrary::SetTriggerPreset(APlayerController* Player, ESteamDualSenseTrigger Trigger, ESteamDualSenseTriggerPreset Preset, float Duration)
{
	SetTriggerEffect(Player, Trigger, MakeTriggerEffectFromPreset(Preset), Duration);
}

FSteamDualSenseTriggerEffect USteamInputFeedbackLibrary::MakeTriggerEffectFromPreset(ESteamDualSenseTriggerPreset Preset)
{
	FSteamDualSenseTriggerEffect Effect;
	switch (Preset)
	{
	case ESteamDualSenseTriggerPreset::Light:
		Effect.Mode = ESteamDualSenseTriggerMode::Feedback;
		Effect.StartPosition = 2;
		Effect.Strength = 3;
		break;
	case ESteamDualSenseTriggerPreset::Heavy:
		Effect.Mode = ESteamDualSenseTriggerMode::Feedback;
		Effect.StartPosition = 0;
		Effect.Strength = 6;
		break;
	case ESteamDualSenseTriggerPreset::Locked:
		Effect.Mode = ESteamDualSenseTriggerMode::Feedback;
		Effect.StartPosition = 0;
		Effect.Strength = 8;
		break;
	case ESteamDualSenseTriggerPreset::Weapon:
		Effect.Mode = ESteamDualSenseTriggerMode::Weapon;
		Effect.StartPosition = 3;
		Effect.EndPosition = 6;
		Effect.Strength = 7;
		break;
	case ESteamDualSenseTriggerPreset::Ramp:
		Effect.Mode = ESteamDualSenseTriggerMode::SlopeFeedback;
		Effect.StartPosition = 0;
		Effect.EndPosition = 9;
		Effect.Strength = 1;
		Effect.EndStrength = 8;
		break;
	case ESteamDualSenseTriggerPreset::Vibration:
		Effect.Mode = ESteamDualSenseTriggerMode::Vibration;
		Effect.StartPosition = 0;
		Effect.Strength = 6;
		Effect.Frequency = 40;
		break;
	case ESteamDualSenseTriggerPreset::Off:
	default:
		break;
	}
	return Effect;
}

void USteamInputFeedbackLibrary::ResetTriggerEffects(APlayerController* Player)
{
	if (USteamInputFeedbackSubsystem* Feedback = USteamInputFeedbackSubsystem::Get(Player))
	{
		Feedback->ResetTriggerEffects(USteamInputFeedbackSubsystem::GetGamepadIndex(Player));
	}
}

void USteamInputFeedbackLibrary::ResetAllTriggerEffects(const UObject* WorldContext)
{
	if (USteamInputFeedbackSubsystem* Feedback = USteamInputFeedbackSubsystem::Get(WorldContext))
	{
		Feedback->ResetAllTriggerEffects();
	}
}

bool USteamInputFeedbackLibrary::HasAdaptiveTriggers(APlayerController* Player)
{
	const int32 GamepadIndex = USteamInputFeedbackSubsystem::GetGamepadIndex(Player);
	const USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
	return GamepadIndex >= 0 && Steam && Steam->HasAdaptiveTriggers(GamepadIndex);
}

int32 USteamInputFeedbackLibrary::GetSteamGamepadIndex(APlayerController* Player)
{
	return USteamInputFeedbackSubsystem::GetGamepadIndex(Player);
}

void USteamInputFeedbackLibrary::SetLightbarColor(APlayerController* Player, FLinearColor Color, float Duration)
{
	if (USteamInputFeedbackSubsystem* Feedback = USteamInputFeedbackSubsystem::Get(Player))
	{
		Feedback->SetLightbarColor(USteamInputFeedbackSubsystem::GetGamepadIndex(Player), Color, Duration);
	}
}

void USteamInputFeedbackLibrary::ResetLightbar(APlayerController* Player)
{
	if (USteamInputFeedbackSubsystem* Feedback = USteamInputFeedbackSubsystem::Get(Player))
	{
		Feedback->ResetLightbar(USteamInputFeedbackSubsystem::GetGamepadIndex(Player));
	}
}

void USteamInputFeedbackLibrary::ResetAllLightbars(const UObject* WorldContext)
{
	if (USteamInputFeedbackSubsystem* Feedback = USteamInputFeedbackSubsystem::Get(WorldContext))
	{
		Feedback->ResetAllLightbars();
	}
}

bool USteamInputFeedbackLibrary::HasLightbar(APlayerController* Player)
{
	const int32 GamepadIndex = USteamInputFeedbackSubsystem::GetGamepadIndex(Player);
	const USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
	if (GamepadIndex < 0 || !Steam)
	{
		return false;
	}
	const ESteamControllerType Type = Steam->GetControllerTypeForGamepadIndex(GamepadIndex);
	return Type == ESteamControllerType::PS4 || Type == ESteamControllerType::PS5;
}

// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputFeedbackSubsystem.h"
#include "SteamInputSubsystem.h"
#include "SteamInputFeedbackState.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamInputFeedback, Log, All);

USteamInputFeedbackSubsystem::USteamInputFeedbackSubsystem() = default;
USteamInputFeedbackSubsystem::~USteamInputFeedbackSubsystem() = default;

USteamInputFeedbackSubsystem* USteamInputFeedbackSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<USteamInputFeedbackSubsystem>() : nullptr;
}

int32 USteamInputFeedbackSubsystem::GetGamepadIndex(const APlayerController* Player)
{
	const ULocalPlayer* LocalPlayer = Player ? Player->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetControllerId() : INDEX_NONE;
}

void USteamInputFeedbackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Pads = MakePimpl<SteamInputSimple::FPadFeedbackMap>();
	if (USteamInputSubsystem* Steam = USteamInputSubsystem::Get())
	{
		ControllersChangedHandle = Steam->OnControllersChangedNative.AddUObject(this, &USteamInputFeedbackSubsystem::HandleControllersChanged);
	}
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &USteamInputFeedbackSubsystem::HandleWorldCleanup);
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &USteamInputFeedbackSubsystem::Tick));
}

void USteamInputFeedbackSubsystem::Deinitialize()
{
	ResetAllTriggerEffects();
	ResetAllLightbars();
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	if (USteamInputSubsystem* Steam = USteamInputSubsystem::Get())
	{
		Steam->OnControllersChangedNative.Remove(ControllersChangedHandle);
	}
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
	Pads.Reset();
	Super::Deinitialize();
}

void USteamInputFeedbackSubsystem::SetTriggerEffect(int32 GamepadIndex, ESteamDualSenseTrigger Trigger, const FSteamDualSenseTriggerEffect& Effect, float Duration)
{
	if (GamepadIndex < 0 || !Pads)
	{
		return;
	}
	SteamInputSimple::FPadFeedback& Pad = Pads->FindOrAdd(GamepadIndex);
	if (Trigger != ESteamDualSenseTrigger::Right)
	{
		Pad.Slots[0].Set(Effect, Duration);
	}
	if (Trigger != ESteamDualSenseTrigger::Left)
	{
		Pad.Slots[1].Set(Effect, Duration);
	}
	// Send now rather than on the next tick: a trigger that locks as the shot fires should not lag a frame.
	Send(GamepadIndex, Pad, bWasSuspended);
}

void USteamInputFeedbackSubsystem::ResetTriggerEffects(int32 GamepadIndex)
{
	if (SteamInputSimple::FPadFeedback* Pad = Pads ? Pads->Find(GamepadIndex) : nullptr)
	{
		Pad->Slots[0].Reset();
		Pad->Slots[1].Reset();
		Send(GamepadIndex, *Pad, bWasSuspended);
		RemoveIfIdle(GamepadIndex);
	}
}

void USteamInputFeedbackSubsystem::SetLightbarColor(int32 GamepadIndex, FLinearColor Color, float Duration)
{
	if (GamepadIndex < 0 || !Pads)
	{
		return;
	}
	SteamInputSimple::FPadFeedback& Pad = Pads->FindOrAdd(GamepadIndex);
	Pad.Light.Set(Color, Duration);
	Send(GamepadIndex, Pad, bWasSuspended);
}

void USteamInputFeedbackSubsystem::ResetLightbar(int32 GamepadIndex)
{
	if (SteamInputSimple::FPadFeedback* Pad = Pads ? Pads->Find(GamepadIndex) : nullptr)
	{
		Pad->Light.Reset();
		Send(GamepadIndex, *Pad, bWasSuspended);
		RemoveIfIdle(GamepadIndex);
	}
}

void USteamInputFeedbackSubsystem::ResetAllLightbars()
{
	if (!Pads)
	{
		return;
	}
	TArray<int32> Indices;
	Pads->GetKeys(Indices);
	for (int32 GamepadIndex : Indices)
	{
		ResetLightbar(GamepadIndex);
	}
}

void USteamInputFeedbackSubsystem::RemoveIfIdle(int32 GamepadIndex)
{
	const SteamInputSimple::FPadFeedback* Pad = Pads ? Pads->Find(GamepadIndex) : nullptr;
	if (Pad && Pad->IsIdle())
	{
		Pads->Remove(GamepadIndex);
	}
}

void USteamInputFeedbackSubsystem::ResetAllTriggerEffects()
{
	if (!Pads)
	{
		return;
	}
	TArray<int32> Indices;
	Pads->GetKeys(Indices);
	for (int32 GamepadIndex : Indices)
	{
		ResetTriggerEffects(GamepadIndex);
	}
}

bool USteamInputFeedbackSubsystem::IsSuspended() const
{
	if (!bReleaseTriggersWhilePaused)
	{
		return false;
	}
	const UGameInstance* GameInstance = GetGameInstance();
	const UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;
	const USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
	return (World && World->IsPaused()) || (Steam && Steam->IsOverlayActive());
}

void USteamInputFeedbackSubsystem::Send(int32 GamepadIndex, SteamInputSimple::FPadFeedback& Pad, bool bSuspended)
{
	USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
	TOptional<FLinearColor> Color;
	if (Pad.TakeLightChange(Color) && Steam)
	{
		// Unset: restore the colour the player chose in Steam.
		Steam->SetLightbarColor(GamepadIndex, Color.Get(FLinearColor::Black), !Color.IsSet());
	}

	TOptional<FSteamDualSenseTriggerEffect> Left, Right;
	Pad.TakeChanges(bSuspended, Left, Right);
	if (!Left.IsSet() && !Right.IsSet())
	{
		return;
	}
	if (Steam)
	{
		Steam->SetDualSenseTriggerEffects(GamepadIndex, Left.GetPtrOrNull(), Right.GetPtrOrNull());
	}
	UE_LOG(LogSteamInputFeedback, Verbose, TEXT("Pad %d triggers: left %s, right %s"), GamepadIndex,
		Left.IsSet() ? *UEnum::GetValueAsString(Left->Mode) : TEXT("unchanged"),
		Right.IsSet() ? *UEnum::GetValueAsString(Right->Mode) : TEXT("unchanged"));
}

bool USteamInputFeedbackSubsystem::Tick(float DeltaTime)
{
	const bool bSuspended = IsSuspended();
	bWasSuspended = bSuspended;
	if (!Pads || Pads->IsEmpty())
	{
		return true;
	}
	for (auto It = Pads->CreateIterator(); It; ++It)
	{
		SteamInputSimple::FPadFeedback& Pad = It.Value();
		if (!bSuspended)
		{
			Pad.Slots[0].Advance(DeltaTime);
			Pad.Slots[1].Advance(DeltaTime);
		}
		Pad.Light.Advance(DeltaTime);
		Send(It.Key(), Pad, bSuspended);
		// Triggers off, default lightbar and nothing pending: forget the pad (Steam already has all of that).
		if (Pad.IsIdle() && !bSuspended)
		{
			It.RemoveCurrent();
		}
	}
	return true;
}

void USteamInputFeedbackSubsystem::HandleControllersChanged()
{
	// A pad that (re)connects starts with its triggers off and its default colour; send everything again on the next tick.
	if (!Pads)
	{
		return;
	}
	for (TPair<int32, SteamInputSimple::FPadFeedback>& Pair : *Pads)
	{
		Pair.Value.ForgetSent();
	}
}

void USteamInputFeedbackSubsystem::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (World && World->IsGameWorld() && World->GetGameInstance() == GetGameInstance() && Pads && !Pads->IsEmpty())
	{
		// Lightbar colours stay: they usually identify the player across levels.
		UE_LOG(LogSteamInputFeedback, Log, TEXT("World %s cleaned up: releasing adaptive triggers."), *World->GetName());
		ResetAllTriggerEffects();
	}
}

// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputFeedbackSubsystem.h"
#include "SteamInputSubsystem.h"
#include "SteamInputFeedbackState.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamInputFeedback, Log, All);

namespace SteamInputFeedbackConsole
{
	/** Straight to Steam, bypassing the subsystem: tells "Steam ignores the colour" apart from "something resets it". */
	static FAutoConsoleCommand SetLightbarCommand(
		TEXT("SteamInput.SetLightbar"),
		TEXT("SteamInput.SetLightbar <GamepadIndex> <R> <G> <B> (0-255), or SteamInput.SetLightbar <GamepadIndex> default"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
			if (!Steam || Args.Num() < 2)
			{
				UE_LOG(LogSteamInputFeedback, Warning, TEXT("Usage: SteamInput.SetLightbar <GamepadIndex> <R> <G> <B> | default"));
				return;
			}
			const int32 GamepadIndex = FCString::Atoi(*Args[0]);
			const bool bDefault = Args[1].Equals(TEXT("default"), ESearchCase::IgnoreCase);
			const FColor Color(bDefault ? 0 : FCString::Atoi(*Args[1]), Args.IsValidIndex(2) ? FCString::Atoi(*Args[2]) : 0,
				Args.IsValidIndex(3) ? FCString::Atoi(*Args[3]) : 0);
			Steam->SetLightbarColor(GamepadIndex, FLinearColor::FromSRGBColor(Color), bDefault);
			UE_LOG(LogSteamInputFeedback, Log, TEXT("SteamInput.SetLightbar: pad %d (handle %lld, %s) -> %s"), GamepadIndex,
				Steam->GetHandleForGamepadIndex(GamepadIndex), *UEnum::GetValueAsString(Steam->GetControllerTypeForGamepadIndex(GamepadIndex)),
				bDefault ? TEXT("Steam default") : *Color.ToString());
		}));
}

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
		ControllersChangedHandle = Steam->OnControllersChangedNative.AddUObject(this, &USteamInputFeedbackSubsystem::ScheduleResync);
		OverlayHandle = Steam->OnOverlayActivatedNative.AddUObject(this, &USteamInputFeedbackSubsystem::HandleOverlay);
	}
	// Steam switches a pad's config (and re-applies its own LED colour) when the game gains or loses focus. Steam's API
	// reports no disconnect then, but UE's virtual pads do reconnect.
	DeviceConnectionHandle = IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddUObject(this, &USteamInputFeedbackSubsystem::HandleDeviceConnectionChange);
	if (FSlateApplication::IsInitialized())
	{
		ReactivatedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &USteamInputFeedbackSubsystem::HandleAppActivation);
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
		Steam->OnOverlayActivatedNative.Remove(OverlayHandle);
	}
	IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().Remove(DeviceConnectionHandle);
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().Remove(ReactivatedHandle);
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
	if (Duration <= 0.f)
	{
		const FColor Requested = Color.ToFColor(true);
		UE_LOG(LogSteamInputFeedback, Log, TEXT("Lightbar requested for pad %d: R=%d G=%d B=%d"), GamepadIndex, Requested.R, Requested.G, Requested.B);
	}
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
	const int64 Handle = Steam ? Steam->GetHandleForGamepadIndex(GamepadIndex) : 0;
	if (!Handle)
	{
		// No pad at this index yet (Steam hasn't enumerated it, or it's a keyboard player). Keep everything pending so
		// it's sent as soon as the pad shows up, instead of counting a call that went nowhere as sent.
		return;
	}

	TOptional<FLinearColor> Color;
	if (Pad.TakeLightChange(Color))
	{
		// Unset: restore the colour the player chose in Steam.
		Steam->SetLightbarColor(GamepadIndex, Color.Get(FLinearColor::Black), !Color.IsSet());
		const FColor Sent = Color.Get(FLinearColor::Black).ToFColor(true);
		UE_LOG(LogSteamInputFeedback, Log, TEXT("Pad %d (handle %lld, %s): lightbar %s"), GamepadIndex, Handle,
			*UEnum::GetValueAsString(Steam->GetControllerTypeForGamepadIndex(GamepadIndex)),
			Color.IsSet() ? *FString::Printf(TEXT("R=%d G=%d B=%d"), Sent.R, Sent.G, Sent.B) : TEXT("Steam default"));
	}

	TOptional<FSteamDualSenseTriggerEffect> Left, Right;
	Pad.TakeChanges(bSuspended, Left, Right);
	if (!Left.IsSet() && !Right.IsSet())
	{
		return;
	}
	Steam->SetDualSenseTriggerEffects(GamepadIndex, Left.GetPtrOrNull(), Right.GetPtrOrNull());
	UE_LOG(LogSteamInputFeedback, Verbose, TEXT("Pad %d triggers: left %s, right %s"), GamepadIndex,
		Left.IsSet() ? *UEnum::GetValueAsString(Left->Mode) : TEXT("unchanged"),
		Right.IsSet() ? *UEnum::GetValueAsString(Right->Mode) : TEXT("unchanged"));
}

bool USteamInputFeedbackSubsystem::Tick(float DeltaTime)
{
	const bool bSuspended = IsSuspended();
	bWasSuspended = bSuspended;
	bool bResync = false;
	for (int32 i = ResyncDelays.Num() - 1; i >= 0; --i)
	{
		ResyncDelays[i] -= DeltaTime;
		if (ResyncDelays[i] <= 0.f)
		{
			ResyncDelays.RemoveAtSwap(i);
			bResync = true;
		}
	}
	if (bResync)
	{
		HandleControllersChanged();
	}
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

void USteamInputFeedbackSubsystem::ScheduleResync()
{
	HandleControllersChanged();
	ResyncDelays = { 0.5f, 2.f };
}

void USteamInputFeedbackSubsystem::HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	ScheduleResync();
}

void USteamInputFeedbackSubsystem::HandleOverlay(bool bActive)
{
	if (!bActive)
	{
		ScheduleResync();
	}
}

void USteamInputFeedbackSubsystem::HandleAppActivation(bool bActive)
{
	if (bActive)
	{
		ScheduleResync();
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

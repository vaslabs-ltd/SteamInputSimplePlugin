// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputPromptSubsystem.h"
#include "SteamInputPromptSettings.h"
#include "SteamInputSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamInputPrompts, Log, All);

namespace
{
	ESteamInputGlyphFamily ToFamily(ESteamControllerType Type)
	{
		switch (Type)
		{
		case ESteamControllerType::PS3:
		case ESteamControllerType::PS4:
		case ESteamControllerType::PS5:
			return ESteamInputGlyphFamily::PlayStation;
		case ESteamControllerType::SwitchPro:
		case ESteamControllerType::SwitchJoyCon:
			return ESteamInputGlyphFamily::Switch;
		case ESteamControllerType::SwitchJoyConSingle:
			return ESteamInputGlyphFamily::SwitchJoyCon;
		case ESteamControllerType::SteamDeck:
			return ESteamInputGlyphFamily::SteamDeck;
		case ESteamControllerType::SteamController:
			return ESteamInputGlyphFamily::SteamController;
		default:
			return ESteamInputGlyphFamily::Xbox;
		}
	}

	ESteamInputPromptButton SwapFace(ESteamInputPromptButton Button)
	{
		switch (Button)
		{
		case ESteamInputPromptButton::A: return ESteamInputPromptButton::B;
		case ESteamInputPromptButton::B: return ESteamInputPromptButton::A;
		case ESteamInputPromptButton::X: return ESteamInputPromptButton::Y;
		case ESteamInputPromptButton::Y: return ESteamInputPromptButton::X;
		default: return Button;
		}
	}
}

USteamInputPromptSubsystem* USteamInputPromptSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<USteamInputPromptSubsystem>() : nullptr;
}

void USteamInputPromptSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (USteamInputSubsystem* Steam = USteamInputSubsystem::Get())
	{
		SteamControllersHandle = Steam->OnControllersChangedNative.AddUObject(this, &USteamInputPromptSubsystem::HandleControllersChanged);
	}
	DeviceConnectionHandle = IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddUObject(this, &USteamInputPromptSubsystem::HandleDeviceConnectionChange);
}

void USteamInputPromptSubsystem::Deinitialize()
{
	if (USteamInputSubsystem* Steam = USteamInputSubsystem::Get())
	{
		Steam->OnControllersChangedNative.Remove(SteamControllersHandle);
	}
	IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().Remove(DeviceConnectionHandle);
	if (DeferredBroadcastHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DeferredBroadcastHandle);
		DeferredBroadcastHandle.Reset();
	}
	Super::Deinitialize();
}

void USteamInputPromptSubsystem::HandleDeviceConnectionChange(EInputDeviceConnectionState NewState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	HandleControllersChanged();
}

void USteamInputPromptSubsystem::HandleControllersChanged()
{
	// Steam and the platform report the same plug/unplug separately and Steam may assign the emulated gamepad index
	// a moment later, so broadcast once, shortly after.
	if (DeferredBroadcastHandle.IsValid())
	{
		return;
	}
	DeferredBroadcastHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
	{
		DeferredBroadcastHandle.Reset();
		LogFamilies();
		OnControllerFamiliesChangedNative.Broadcast();
		OnControllerFamiliesChanged.Broadcast();
		return false;
	}), 0.1f);
}

void USteamInputPromptSubsystem::LogFamilies() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		for (const ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
		{
			if (LocalPlayer)
			{
				UE_LOG(LogSteamInputPrompts, Log, TEXT("Local player %s (ControllerId %d): %s"), *LocalPlayer->GetName(), LocalPlayer->GetControllerId(),
					*UEnum::GetValueAsString(GetFamilyForLocalPlayer(LocalPlayer)));
			}
		}
	}
}

ESteamInputGlyphFamily USteamInputPromptSubsystem::GetFamilyForControllerId(int32 ControllerId) const
{
	const USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
	return Steam ? ToFamily(Steam->GetControllerTypeForGamepadIndex(ControllerId)) : ESteamInputGlyphFamily::Xbox;
}

ESteamInputGlyphFamily USteamInputPromptSubsystem::GetFamilyForLocalPlayer(const ULocalPlayer* LocalPlayer) const
{
	return LocalPlayer ? GetFamilyForControllerId(LocalPlayer->GetControllerId()) : ESteamInputGlyphFamily::Xbox;
}

const ULocalPlayer* USteamInputPromptSubsystem::GetPromptPlayer(const APlayerController* Owner) const
{
	if (const ULocalPlayer* OwnerPlayer = Owner ? Owner->GetLocalPlayer() : nullptr)
	{
		return OwnerPlayer;
	}
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetFirstGamePlayer() : nullptr;
}

ESteamInputGlyphFamily USteamInputPromptSubsystem::GetPromptFamily(const APlayerController* Owner) const
{
	return GetFamilyForLocalPlayer(GetPromptPlayer(Owner));
}

bool USteamInputPromptSubsystem::ShouldSwapFaceButtons(int32 ControllerId, ESteamInputGlyphFamily Family) const
{
	if (Family != ESteamInputGlyphFamily::Switch)
	{
		return false;
	}
	switch (USteamInputPromptSettings::Get().SwitchLayout)
	{
	case ESteamInputSwitchLayout::Labels: return false;
	case ESteamInputSwitchLayout::Positions: return true;
	default:
	{
		const USteamInputSubsystem* Steam = USteamInputSubsystem::Get();
		return Steam && !Steam->UsesNintendoButtonLabels(ControllerId);
	}
	}
}

FText USteamInputPromptSubsystem::FormatPrompt(const FText& Template, ESteamInputGlyphFamily Family, bool bSwapFaceButtons)
{
	// Plain token replacement rather than FText::Format: other {0}-style arguments must survive untouched.
	FString Result = Template.ToString();
	if (!Result.Contains(TEXT("{")))
	{
		return Template;
	}
	const USteamInputPromptSettings& Settings = USteamInputPromptSettings::Get();
	const UEnum* Buttons = StaticEnum<ESteamInputPromptButton>();
	for (int32 Index = 0; Index < Buttons->NumEnums() - 1; ++Index)
	{
		const ESteamInputPromptButton Button = static_cast<ESteamInputPromptButton>(Buttons->GetValueByIndex(Index));
		const FString Token = FString::Printf(TEXT("{%s}"), *USteamInputPromptSettings::GetButtonToken(Button));
		if (!Result.Contains(Token, ESearchCase::CaseSensitive))
		{
			continue;
		}
		const ESteamInputPromptButton Shown = bSwapFaceButtons ? SwapFace(Button) : Button;
		ESteamInputGlyphFamily GlyphFamily = Family;
		const FSteamInputPromptGlyph* Glyph = Settings.FindGlyph(Family, Shown, &GlyphFamily);
		const FString Markup = Glyph
			? FString::Printf(TEXT("<%s>%s</>"), *Settings.GetStyleRow(GlyphFamily, Shown, *Glyph).ToString(), *Glyph->Text)
			: USteamInputPromptSettings::GetButtonToken(Shown);
		Result.ReplaceInline(*Token, *Markup, ESearchCase::CaseSensitive);
	}
	return FText::FromString(MoveTemp(Result));
}

FText USteamInputPromptSubsystem::FormatPromptFor(const FText& Template, const APlayerController* Owner) const
{
	const ULocalPlayer* Player = GetPromptPlayer(Owner);
	const ESteamInputGlyphFamily Family = GetFamilyForLocalPlayer(Player);
	return FormatPrompt(Template, Family, Player && ShouldSwapFaceButtons(Player->GetControllerId(), Family));
}

UTexture2D* USteamInputPromptSubsystem::GetTextureForFamily(UTexture2D* XboxTexture, ESteamInputGlyphFamily Family)
{
	if (!XboxTexture || Family == ESteamInputGlyphFamily::Xbox)
	{
		return XboxTexture;
	}
	const FSteamInputImageVariants* Variants = USteamInputPromptSettings::Get().Images.Find(TSoftObjectPtr<UTexture2D>(XboxTexture));
	const TSoftObjectPtr<UTexture2D>* Variant = Variants ? Variants->Variants.Find(Family) : nullptr;
	UTexture2D* Loaded = Variant ? Variant->LoadSynchronous() : nullptr;
	return Loaded ? Loaded : XboxTexture;
}

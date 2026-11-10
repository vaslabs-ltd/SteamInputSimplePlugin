// Copyright Vaslabs LTD 2026 All Rights Reserved.

#include "SteamInputFeedbackLibrary.h"
#include "SteamInputSimpleInternal.h"
#include "SteamInputFeedbackState.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace SteamInputFeedbackTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter;

	FSteamDualSenseTriggerEffect Preset(ESteamDualSenseTriggerPreset P)
	{
		return USteamInputFeedbackLibrary::MakeTriggerEffectFromPreset(P);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSteamInputTriggerSlotTest, "SteamInputSimplePlugin.Feedback.TriggerSlot", SteamInputFeedbackTests::Flags)

bool FSteamInputTriggerSlotTest::RunTest(const FString& Parameters)
{
	using namespace SteamInputFeedbackTests;
	const FSteamDualSenseTriggerEffect Off, Heavy = Preset(ESteamDualSenseTriggerPreset::Heavy), Locked = Preset(ESteamDualSenseTriggerPreset::Locked);

	SteamInputSimple::FTriggerSlot Slot;
	TestTrue(TEXT("New slot is idle"), Slot.IsIdle());

	Slot.Set(Heavy, 0.f);
	TestTrue(TEXT("Lasting effect is current"), Slot.Current() == Heavy);
	TestFalse(TEXT("Lasting effect is not idle"), Slot.IsIdle());

	Slot.Set(Locked, 1.f);
	TestTrue(TEXT("Timed effect wins"), Slot.Current() == Locked);
	Slot.Advance(0.6f);
	TestTrue(TEXT("Timed effect still on"), Slot.Current() == Locked);
	Slot.Advance(0.6f);
	TestTrue(TEXT("Back to lasting after the timed effect"), Slot.Current() == Heavy);

	Slot.Set(Locked, 1.f);
	Slot.Set(Off, 0.f);
	TestTrue(TEXT("Setting a lasting effect cancels the timed one"), Slot.Current() == Off && Slot.IsIdle());

	Slot.Set(Locked, 0.5f);
	Slot.Set(Locked, 2.f);
	Slot.Advance(1.f);
	TestTrue(TEXT("A new timed effect restarts the timer"), Slot.Current() == Locked);
	Slot.Advance(1.f);
	TestTrue(TEXT("Timed effect over, Off by default"), Slot.IsIdle());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSteamInputPadTriggersTest, "SteamInputSimplePlugin.Feedback.PadTriggers", SteamInputFeedbackTests::Flags)

bool FSteamInputPadTriggersTest::RunTest(const FString& Parameters)
{
	using namespace SteamInputFeedbackTests;
	const FSteamDualSenseTriggerEffect Off, Weapon = Preset(ESteamDualSenseTriggerPreset::Weapon);
	SteamInputSimple::FPadFeedback Pad;
	TOptional<FSteamDualSenseTriggerEffect> Left, Right;

	Pad.TakeChanges(false, Left, Right);
	TestTrue(TEXT("First send covers both triggers"), Left.IsSet() && Right.IsSet() && *Left == Off && *Right == Off);
	Pad.TakeChanges(false, Left, Right);
	TestTrue(TEXT("Nothing re-sent when unchanged"), !Left.IsSet() && !Right.IsSet());

	Pad.Slots[1].Set(Weapon, 0.f);
	Pad.TakeChanges(false, Left, Right);
	TestTrue(TEXT("Only the changed trigger is sent"), !Left.IsSet() && Right.IsSet() && *Right == Weapon);

	Pad.TakeChanges(true, Left, Right);
	TestTrue(TEXT("Paused: right released, left already off"), !Left.IsSet() && Right.IsSet() && *Right == Off);
	Pad.TakeChanges(false, Left, Right);
	TestTrue(TEXT("Unpaused: effect restored"), !Left.IsSet() && Right.IsSet() && *Right == Weapon);

	Pad.ForgetSent();
	Pad.TakeChanges(false, Left, Right);
	TestTrue(TEXT("Reconnect: both re-sent"), Left.IsSet() && Right.IsSet() && *Right == Weapon);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSteamInputLightbarTest, "SteamInputSimplePlugin.Feedback.Lightbar", SteamInputFeedbackTests::Flags)

bool FSteamInputLightbarTest::RunTest(const FString& Parameters)
{
	SteamInputSimple::FPadFeedback Pad;
	TOptional<FLinearColor> Color;

	TestFalse(TEXT("Nothing set: no call, the pad keeps its default"), Pad.TakeLightChange(Color));
	TestTrue(TEXT("Untouched pad is idle"), Pad.IsIdle());

	Pad.Light.Set(FLinearColor::Red, 0.f);
	TestTrue(TEXT("Lasting colour sent"), Pad.TakeLightChange(Color) && Color.IsSet() && *Color == FLinearColor::Red);
	TestFalse(TEXT("Not re-sent when unchanged"), Pad.TakeLightChange(Color));
	TestFalse(TEXT("A lasting colour is not idle"), Pad.IsIdle());

	Pad.Light.Set(FLinearColor::White, 0.5f);
	TestTrue(TEXT("Flash sent"), Pad.TakeLightChange(Color) && *Color == FLinearColor::White);
	Pad.Light.Advance(0.6f);
	TestTrue(TEXT("Back to the lasting colour after the flash"), Pad.TakeLightChange(Color) && *Color == FLinearColor::Red);

	Pad.ForgetSent();
	TestTrue(TEXT("Reconnect: colour re-sent"), Pad.TakeLightChange(Color) && *Color == FLinearColor::Red);

	Pad.Light.Reset();
	TestTrue(TEXT("Reset sends Steam's default"), Pad.TakeLightChange(Color) && !Color.IsSet());
	TestTrue(TEXT("Reset pad is idle"), Pad.IsIdle());

	SteamInputSimple::FPadFeedback Flashed;
	Flashed.Light.Set(FLinearColor::Blue, 0.2f);
	Flashed.TakeLightChange(Color);
	Flashed.Light.Advance(0.3f);
	TestTrue(TEXT("Flash with no lasting colour ends on Steam's default"), Flashed.TakeLightChange(Color) && !Color.IsSet());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSteamInputTriggerCommandTest, "SteamInputSimplePlugin.Feedback.TriggerCommand", SteamInputFeedbackTests::Flags)

bool FSteamInputTriggerCommandTest::RunTest(const FString& Parameters)
{
	using namespace SteamInputFeedbackTests;
	ScePadTriggerEffectCommand Cmd;

	SteamInputSimple::FillTriggerCommand(Preset(ESteamDualSenseTriggerPreset::Off), Cmd);
	TestEqual(TEXT("Off"), (int32)Cmd.mode, (int32)SCE_PAD_TRIGGER_EFFECT_MODE_OFF);

	SteamInputSimple::FillTriggerCommand(Preset(ESteamDualSenseTriggerPreset::Locked), Cmd);
	TestEqual(TEXT("Locked is feedback"), (int32)Cmd.mode, (int32)SCE_PAD_TRIGGER_EFFECT_MODE_FEEDBACK);
	TestEqual(TEXT("Locked from position 0"), (int32)Cmd.commandData.feedbackParam.position, 0);
	TestEqual(TEXT("Locked at full strength"), (int32)Cmd.commandData.feedbackParam.strength, 8);

	SteamInputSimple::FillTriggerCommand(Preset(ESteamDualSenseTriggerPreset::Weapon), Cmd);
	TestEqual(TEXT("Weapon"), (int32)Cmd.mode, (int32)SCE_PAD_TRIGGER_EFFECT_MODE_WEAPON);
	TestTrue(TEXT("Weapon range valid"), Cmd.commandData.weaponParam.startPosition >= 2 && Cmd.commandData.weaponParam.startPosition <= 7
		&& Cmd.commandData.weaponParam.endPosition > Cmd.commandData.weaponParam.startPosition && Cmd.commandData.weaponParam.endPosition <= 8);

	SteamInputSimple::FillTriggerCommand(Preset(ESteamDualSenseTriggerPreset::Ramp), Cmd);
	TestEqual(TEXT("Ramp is slope"), (int32)Cmd.mode, (int32)SCE_PAD_TRIGGER_EFFECT_MODE_SLOPE_FEEDBACK);
	TestEqual(TEXT("Ramp start strength"), (int32)Cmd.commandData.slopeFeedbackParam.startStrength, 1);
	TestEqual(TEXT("Ramp end strength"), (int32)Cmd.commandData.slopeFeedbackParam.endStrength, 8);
	TestEqual(TEXT("Ramp end position"), (int32)Cmd.commandData.slopeFeedbackParam.endPosition, 9);

	// Out-of-range values are clamped to what the mode accepts.
	FSteamDualSenseTriggerEffect Bad;
	Bad.Mode = ESteamDualSenseTriggerMode::Weapon;
	Bad.StartPosition = 0;
	Bad.EndPosition = 0;
	Bad.Strength = 99;
	SteamInputSimple::FillTriggerCommand(Bad, Cmd);
	TestEqual(TEXT("Weapon start clamped"), (int32)Cmd.commandData.weaponParam.startPosition, 2);
	TestEqual(TEXT("Weapon end after start"), (int32)Cmd.commandData.weaponParam.endPosition, 3);
	TestEqual(TEXT("Strength clamped"), (int32)Cmd.commandData.weaponParam.strength, 8);

	Bad.Mode = ESteamDualSenseTriggerMode::SlopeFeedback;
	Bad.StartPosition = 9;
	Bad.Strength = 0;
	Bad.EndStrength = 0;
	SteamInputSimple::FillTriggerCommand(Bad, Cmd);
	TestEqual(TEXT("Slope start clamped"), (int32)Cmd.commandData.slopeFeedbackParam.startPosition, 8);
	TestEqual(TEXT("Slope end after start"), (int32)Cmd.commandData.slopeFeedbackParam.endPosition, 9);
	TestEqual(TEXT("Slope strengths at least 1"), (int32)Cmd.commandData.slopeFeedbackParam.startStrength + Cmd.commandData.slopeFeedbackParam.endStrength, 2);

	FSteamDualSenseTriggerEffect Zones;
	Zones.Mode = ESteamDualSenseTriggerMode::MultiplePositionFeedback;
	Zones.ZoneStrengths = { 0, 2, 12 };
	SteamInputSimple::FillTriggerCommand(Zones, Cmd);
	TestEqual(TEXT("Zones mode"), (int32)Cmd.mode, (int32)SCE_PAD_TRIGGER_EFFECT_MODE_MULTIPLE_POSITION_FEEDBACK);
	TestEqual(TEXT("Zone 1"), (int32)Cmd.commandData.multiplePositionFeedbackParam.strength[1], 2);
	TestEqual(TEXT("Zone 2 clamped"), (int32)Cmd.commandData.multiplePositionFeedbackParam.strength[2], 8);
	TestEqual(TEXT("Missing zone is 0"), (int32)Cmd.commandData.multiplePositionFeedbackParam.strength[9], 0);
	return true;
}

#endif

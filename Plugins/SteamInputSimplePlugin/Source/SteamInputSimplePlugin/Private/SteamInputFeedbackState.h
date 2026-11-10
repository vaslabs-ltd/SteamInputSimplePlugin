// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SteamInputTypes.h"

namespace SteamInputSimple
{
	/**
	 * One adaptive trigger of one pad: a lasting effect, and an optional timed effect on top of it that falls back to
	 * the lasting one when it runs out. Plain C++ so it can be tested without Steam.
	 */
	struct FTriggerSlot
	{
		/** Stays until replaced or reset. */
		FSteamDualSenseTriggerEffect Lasting;
		/** Wins over Lasting while TimedRemaining > 0. */
		FSteamDualSenseTriggerEffect Timed;
		float TimedRemaining = 0.f;

		bool HasTimed() const { return TimedRemaining > 0.f; }
		const FSteamDualSenseTriggerEffect& Current() const { return HasTimed() ? Timed : Lasting; }
		bool IsIdle() const { return !HasTimed() && Lasting.Mode == ESteamDualSenseTriggerMode::Off; }

		/** Duration <= 0: lasting (also cancels a running timed effect). Duration > 0: timed, replacing any timed one. */
		void Set(const FSteamDualSenseTriggerEffect& Effect, float Duration)
		{
			if (Duration > 0.f)
			{
				Timed = Effect;
				TimedRemaining = Duration;
			}
			else
			{
				Lasting = Effect;
				TimedRemaining = 0.f;
			}
		}

		void Advance(float DeltaTime)
		{
			if (HasTimed())
			{
				TimedRemaining = FMath::Max(0.f, TimedRemaining - DeltaTime);
			}
		}

		void Reset() { *this = FTriggerSlot(); }
	};

	/**
	 * The lightbar of one pad: a lasting colour (unset = the player's Steam default), and an optional timed colour on top
	 * (a flash) that falls back to the lasting one.
	 */
	struct FLightbarSlot
	{
		TOptional<FLinearColor> Lasting;
		FLinearColor Timed = FLinearColor::Black;
		float TimedRemaining = 0.f;

		bool HasTimed() const { return TimedRemaining > 0.f; }
		/** Unset: Steam's default colour for this pad. */
		TOptional<FLinearColor> Current() const { return HasTimed() ? TOptional<FLinearColor>(Timed) : Lasting; }
		bool IsIdle() const { return !HasTimed() && !Lasting.IsSet(); }

		/** Duration <= 0: lasting (also cancels a running flash). Duration > 0: a timed flash, replacing any running one. */
		void Set(const FLinearColor& Color, float Duration)
		{
			if (Duration > 0.f)
			{
				Timed = Color;
				TimedRemaining = Duration;
			}
			else
			{
				Lasting = Color;
				TimedRemaining = 0.f;
			}
		}

		void Advance(float DeltaTime)
		{
			if (HasTimed())
			{
				TimedRemaining = FMath::Max(0.f, TimedRemaining - DeltaTime);
			}
		}

		void Reset() { *this = FLightbarSlot(); }
	};

	/** Everything the plugin drives on one pad, plus what was last sent to Steam so unchanged values aren't re-sent. */
	struct FPadFeedback
	{
		FTriggerSlot Slots[2]; // 0 = left (L2), 1 = right (R2)
		TOptional<FSteamDualSenseTriggerEffect> Sent[2];

		FLightbarSlot Light;
		/** Outer unset: nothing sent yet. Inner unset: Steam's default colour was sent. */
		TOptional<TOptional<FLinearColor>> SentLight;

		bool AreTriggersIdle() const { return Slots[0].IsIdle() && Slots[1].IsIdle(); }
		bool IsIdle() const { return AreTriggersIdle() && Light.IsIdle(); }
		void ForgetSent() { Sent[0].Reset(); Sent[1].Reset(); SentLight.Reset(); }

		/**
		 * What to send now: Off on both while bSuspended, else each slot's current effect. A trigger is null when Steam
		 * already has that effect. Records the result as sent.
		 */
		void TakeChanges(bool bSuspended, TOptional<FSteamDualSenseTriggerEffect>& OutLeft, TOptional<FSteamDualSenseTriggerEffect>& OutRight)
		{
			TOptional<FSteamDualSenseTriggerEffect>* Outs[2] = { &OutLeft, &OutRight };
			for (int32 i = 0; i < 2; ++i)
			{
				const FSteamDualSenseTriggerEffect Wanted = bSuspended ? FSteamDualSenseTriggerEffect() : Slots[i].Current();
				Outs[i]->Reset();
				if (!Sent[i].IsSet() || Sent[i].GetValue() != Wanted)
				{
					*Outs[i] = Wanted;
					Sent[i] = Wanted;
				}
			}
		}

		/** True when the lightbar must be sent; OutColor unset means "restore Steam's default". Records it as sent. */
		bool TakeLightChange(TOptional<FLinearColor>& OutColor)
		{
			const TOptional<FLinearColor> Wanted = Light.Current();
			if (SentLight.IsSet() && SentLight.GetValue() == Wanted)
			{
				return false;
			}
			// Nothing ever set and nothing sent: leave the pad on its default without a call.
			if (!SentLight.IsSet() && !Wanted.IsSet())
			{
				SentLight.Emplace(Wanted);
				return false;
			}
			OutColor = Wanted;
			SentLight.Emplace(Wanted);
			return true;
		}
	};

	struct FPadFeedbackMap : TMap<int32, FPadFeedback>
	{
	};
}

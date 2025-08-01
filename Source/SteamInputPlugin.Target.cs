// Copyright (c) 2025 Vaslabs LTD. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SteamInputPluginTarget : TargetRules
{
	public SteamInputPluginTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("SteamInputPlugin");
	}
}

// Copyright Vaslabs LTD 2025 All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SteamInputPluginTarget : TargetRules
{
	public SteamInputPluginTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		BuildEnvironment = TargetBuildEnvironment.Unique;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SteamInputPlugin");
	}
}

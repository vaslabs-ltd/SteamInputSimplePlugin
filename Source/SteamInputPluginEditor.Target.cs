// Copyright Vaslabs LTD 2025 All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SteamInputPluginEditorTarget : TargetRules
{
	public SteamInputPluginEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("SteamInputPlugin");
	}
}

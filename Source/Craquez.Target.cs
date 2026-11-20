// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CraquezTarget : TargetRules
{
	public CraquezTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Craquez" });
	}
}

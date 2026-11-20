// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CraquezEditorTarget : TargetRules
{
	public CraquezEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Craquez" });
	}
}

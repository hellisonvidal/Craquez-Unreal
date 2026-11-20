// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;

public class FootballCoreGameplay : ModuleRules
{
	public FootballCoreGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"ControlRig",
			"FootballCore"
		});
	}
}

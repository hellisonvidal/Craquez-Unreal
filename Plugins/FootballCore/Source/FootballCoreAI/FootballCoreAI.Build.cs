// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;

public class FootballCoreAI : ModuleRules
{
	public FootballCoreAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AIModule",
			"GameplayTasks",
			"GameplayTags",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"FootballCore",
			"FootballCoreGameplay"
		});
	}
}

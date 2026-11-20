// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;

public class Craquez : ModuleRules
{
	public Craquez(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"FootballCore",
			"FootballCoreGameplay",
			"FootballCoreAI",
			"FootballCoreNetworking",
			"FootballCoreUI"
		});
	}
}

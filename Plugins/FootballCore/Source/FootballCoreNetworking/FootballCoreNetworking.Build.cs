// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;

public class FootballCoreNetworking : ModuleRules
{
	public FootballCoreNetworking(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"NetCore",
			"GameplayTags",
			"FootballCore",
			"FootballCoreGameplay"
		});
	}
}

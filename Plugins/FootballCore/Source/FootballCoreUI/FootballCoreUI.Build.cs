// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;

public class FootballCoreUI : ModuleRules
{
	public FootballCoreUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"FootballCore",
			"FootballCoreGameplay",
			"FootballCoreNetworking"
		});
	}
}

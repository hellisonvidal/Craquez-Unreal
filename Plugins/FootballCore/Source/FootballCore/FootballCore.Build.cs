// Copyright 2024 Craquez. All Rights Reserved.

using UnrealBuildTool;

public class FootballCore : ModuleRules
{
	public FootballCore(ReadOnlyTargetRules Target) : base(Target)
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
			"PhysicsCore",
			"Niagara",
			"ControlRig"
		});
	}
}

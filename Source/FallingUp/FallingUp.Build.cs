// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FallingUp : ModuleRules
{
	public FallingUp(ReadOnlyTargetRules Target) : base(Target) 
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		if (Target.Type == TargetType.Editor)
		{   
			PrivateDependencyModuleNames.AddRange(new string[] { "UMGEditor" });
		}
	}
}

// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

using UnrealBuildTool;

public class DTSelectFile : ModuleRules
{
	public DTSelectFile(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange( new string[] {} );
		PrivateIncludePaths.AddRange( new string[] {} );
		PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
				}
			);
		PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore"
				}
			);
		DynamicallyLoadedModuleNames.AddRange( new string[]{} );
	}
}

// Copyright 2024 Arkai Interactive. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Chestory : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string PluginPath
	{
		get { return Path.Combine(PluginDirectory, "Source/"); }
	}

	public Chestory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.Add(Path.Combine(PluginPath, "Chestory/Private"));
		PublicIncludePaths.Add(Path.Combine(PluginPath, "Chestory/Public"));

		//PublicDefinitions.Add("CHESTORY_API");
		

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreOnline",
                "Engine",
				"CoreUObject",
				"InputCore",
				"Slate",
				"SlateCore",
				"UMG",
				"AssetRegistry",
				"GameplayTags",
                "EnhancedInput", 
				//"Json",
				//"JsonUtilities",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AssetRegistry",
				"NetCore",
				//"Json",
				//"JsonUtilities",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

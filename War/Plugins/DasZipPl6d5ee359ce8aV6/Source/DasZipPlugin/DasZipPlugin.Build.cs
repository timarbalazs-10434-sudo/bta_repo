// Copyright 2020-2022 MassSun. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using Microsoft.Win32;

public class DasZipPlugin : ModuleRules
{
    private string ModulePath
    {
        get
        {
            return ModuleDirectory;
        }
    }

    private string ThirdPartyPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty"));
        }
    }


    public DasZipPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
            }
            );

        PrivateIncludePaths.Add(Path.Combine(ThirdPartyPath, "minizip"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
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
                "zlib",
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

// Copyright (c) 2025 Picasso Labs. All Rights Reserved.
//
// This file is part of NitroPythonRuntime, licensed under the Picasso Labs Software License.

using UnrealBuildTool;
using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;

public class NitroPythonRuntime : ModuleRules
{
	private string GetPythonDirectoryForEngineVersion(ReadOnlyTargetRules Target)
	{
		// Parse engine version from Target
		int majorVersion = Target.Version.MajorVersion;
		int minorVersion = Target.Version.MinorVersion;
		
		System.Console.WriteLine($"NitroPythonRuntime: Detected UE version {majorVersion}.{minorVersion}");
		
		// Version mapping:
		// UE 5.1, 5.2, 5.3 -> Python39
		// UE 5.4 and up -> Python311
		if (majorVersion == 5)
		{
			if (minorVersion >= 1 && minorVersion <= 3)
			{
				System.Console.WriteLine("NitroPythonRuntime: Using Python39 for UE 5.1-5.3");
				return "Python39";
			}
			else if (minorVersion >= 4)
			{
				System.Console.WriteLine("NitroPythonRuntime: Using Python311 for UE 5.4+");
				return "Python311";
			}
		}
		
		// Default to Python311 for unknown versions (future UE versions)
		System.Console.WriteLine("NitroPythonRuntime: Using Python311 as default for unknown UE version");
		return "Python311";
	}

	public NitroPythonRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableExceptions = true;

        string ThirdPartyPath = Path.Combine(ModuleDirectory, "../ThirdParty/");
		
		// Determine Python version based on Unreal Engine version
		string PythonDirName = GetPythonDirectoryForEngineVersion(Target);
		string PythonLibsPath = Path.Combine(ThirdPartyPath, PythonDirName, "libs");

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(ThirdPartyPath, PythonDirName, "include")
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
				"Projects",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, PythonDirName));

        // Add Python directory (libs/ and include/) as runtime dependency
        string PluginDir = ModuleDirectory;
        string PythonDir = Path.Combine(PluginDir, "..", "..", "Source", "ThirdParty", PythonDirName);
        // Ensure all files in PythonDir are marked as RuntimeDependencies
        foreach (string FilePath in Directory.GetFiles(PythonDir, "*", SearchOption.AllDirectories))
        {
            string RelativePath = FilePath.Replace(PluginDir + Path.DirectorySeparatorChar, "");
            RuntimeDependencies.Add(FilePath, StagedFileType.NonUFS);
        }

		// Add Engine Python directory as runtime dependency
		string EngineRootDir = Path.GetFullPath(Target.RelativeEnginePath);
		string EnginePythonDir = Path.Combine(EngineRootDir, "Binaries", "ThirdParty", "Python3", "Win64");
		System.Console.WriteLine("NitroPythonRuntime: resolved Engine root to: " + EngineRootDir);
		foreach (string FilePath in Directory.GetFiles(EnginePythonDir, "*", SearchOption.AllDirectories))
		{
			if (!File.Exists(FilePath))
			{
				throw new BuildException($"NitroPythonRuntime: File not found: {FilePath}");
			}
			// No need for relative path if you're using NonUFS
			RuntimeDependencies.Add(FilePath, StagedFileType.NonUFS);
		}

		// Automatically detect the highest available Python version library
        string[] pythonLibFiles = Directory.GetFiles(PythonLibsPath, "python3*.lib");

        if (pythonLibFiles.Length > 0)
        {
            string latestPythonLib = pythonLibFiles
                .OrderByDescending(f => f) // Sort to get the highest version
                .FirstOrDefault();

            if (!string.IsNullOrEmpty(latestPythonLib))
            {
                PublicSystemLibraryPaths.Add(PythonLibsPath);
                PublicAdditionalLibraries.Add(latestPythonLib);

                System.Console.WriteLine("NitroPythonRuntime: using Python library: " + latestPythonLib + " from " + PythonDirName);
            }
            else
            {
                System.Console.WriteLine("NitroPythonRuntime: No valid Python library found in " + PythonLibsPath);
            }
        }
        else
        {
            System.Console.WriteLine("NitroPythonRuntime: No Python library found in " + PythonLibsPath);
            throw new Exception("Python library not found. Ensure Python libraries are placed in the correct directory.");
        }

	}
}

// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

using UnrealBuildTool;

public class DTSelectFileEditor : ModuleRules
{
    public DTSelectFileEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject", 
                "Engine", 
                "InputCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "GraphEditor",
                "BlueprintGraph",
                "KismetCompiler",
                "DTSelectFile",
                "ToolMenus",
            }
        );
    }
}
// Copyright Teleograph, LLC. All Rights Reserved.

using UnrealBuildTool;

public class SplinFormationEditor : ModuleRules
{
	public SplinFormationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"CoreUObject", 
				"Engine", 
				"EditorSubsystem",
				"SplinFormationRuntime"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "LevelEditor",    // For adding menu items to Level Editor
				"ToolMenus",      // For FToolMenus system
				"Slate",          // For Slate UI
				"UnrealEd",
				"AssetRegistry",
                "SlateCore",      // Core Slate functionality
				"Landscape"     
				// ... add private dependencies that you statically link with here ...	
			}
			);

        // Only needed if plugin will ever run in editor-only contexts
        if (Target.Type == TargetType.Editor)
        {
            PrivateIncludePathModuleNames.AddRange(new string[]
            {
                
            });
        }

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

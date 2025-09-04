// Copyright Teleograph, LLC. All Rights Reserved.

using UnrealBuildTool;

public class SplinFormationRuntime : ModuleRules
{
	public SplinFormationRuntime(ReadOnlyTargetRules Target) : base(Target)
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
				"Engine"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Landscape"     // Optional: for icons and styles
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

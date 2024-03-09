// Some copyright should be here...

using UnrealBuildTool;

public class Dolt : ModuleRules
{
	public Dolt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "ApplicationCore", "Blutility", "Core", "CoreUObject", "Engine", "InputCore", "DeveloperSettings" });

		PrivateDependencyModuleNames.AddRange(new string[] { "ApplicationCore", "Blutility", "SourceControl"});
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

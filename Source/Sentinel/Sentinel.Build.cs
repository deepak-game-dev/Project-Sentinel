using UnrealBuildTool;

public class Sentinel : ModuleRules
{
	public Sentinel(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay",
			"AIModule", "GameplayTasks", "NavigationSystem", "Slate", "SlateCore"
		});
	}
}

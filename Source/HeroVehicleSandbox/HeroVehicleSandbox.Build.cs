using UnrealBuildTool;

public class HeroVehicleSandbox : ModuleRules
{
    public HeroVehicleSandbox(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "Slate",
            "SlateCore",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks"
        });
    }
}

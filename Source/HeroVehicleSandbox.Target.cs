using UnrealBuildTool;
using System.Collections.Generic;

public class HeroVehicleSandboxTarget : TargetRules
{
    public HeroVehicleSandboxTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("HeroVehicleSandbox");
    }
}

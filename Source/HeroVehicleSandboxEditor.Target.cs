using UnrealBuildTool;
using System.Collections.Generic;

public class HeroVehicleSandboxEditorTarget : TargetRules
{
    public HeroVehicleSandboxEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("HeroVehicleSandbox");
    }
}

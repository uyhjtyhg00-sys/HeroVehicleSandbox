
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$HeroTypesPath = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Public\HeroTypes.h"
$HeroCharacterPath = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroCharacter.cpp"
$SettingsWidgetPath = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroSettingsWidget.cpp"
$InputPath = Join-Path $ProjectRoot "Config\DefaultInput.ini"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

foreach ($Path in @($HeroTypesPath, $HeroCharacterPath, $InputPath)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }
}

function Backup-File([string]$Path) {
    $Backup = "$Path.ow_yaw0066.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

Backup-File $HeroTypesPath
Backup-File $HeroCharacterPath
Backup-File $InputPath
if (Test-Path $SettingsWidgetPath) {
    Backup-File $SettingsWidgetPath
}

# ---------------------------------------------------------------------
# 1) HeroTypes.h: make FHeroAimSettings use Overwatch yaw = 0.0066.
#    Runtime formula:
#      degrees/count = InGameSensitivity * 0.0066
#    Display formula:
#      cm/360 = 360 * 2.54 / (DPI * Sensitivity * 0.0066)
#             ~= 138600 / (DPI * Sensitivity)
# ---------------------------------------------------------------------
$HeroTypes = Get-Content $HeroTypesPath -Raw -Encoding UTF8

$NewAimSettings = @'
USTRUCT(BlueprintType)
struct FHeroAimSettings
{
    GENERATED_BODY()

    // Overwatch-style yaw.
    // Runtime:
    //   degrees/count = MouseSensitivity * 0.0066
    //
    // cm/360:
    //   cm/360 = 360 * 2.54 / (DPI * MouseSensitivity * 0.0066)
    //          ~= 138600 / (DPI * MouseSensitivity)
    static constexpr float OverwatchYawDegreesPerCount = 0.0066f;
    static constexpr float OverwatchCm360Constant = 138600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float ScopedSensitivityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float VehicleLookSensitivityMultiplier = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float FirstPersonFovDegrees = 103.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    bool bInvertMouseY = false;

    // For cm/360 calculation and settings display only.
    // DPI is NOT multiplied into runtime camera rotation; DPI changes the physical
    // distance needed to produce the raw mouse counts.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float ReferenceMouseDpi = 1600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Settings")
    float TargetCmPer360 = 86.6f;

    float GetBaseDegreesPerCount() const
    {
        return OverwatchYawDegreesPerCount;
    }

    float GetFinalDegreesPerCount(const bool bScoped, const bool bVehicle) const
    {
        float Result = OverwatchYawDegreesPerCount * MouseSensitivity;
        Result *= bScoped ? ScopedSensitivityMultiplier : 1.0f;
        Result *= bVehicle ? VehicleLookSensitivityMultiplier : 1.0f;
        return Result;
    }

    float GetCmPer360() const
    {
        const float SafeDpi = FMath::Max(1.0f, ReferenceMouseDpi);
        const float SafeSensitivity = FMath::Max(0.001f, MouseSensitivity);
        return OverwatchCm360Constant / (SafeDpi * SafeSensitivity);
    }

    float GetSensitivityForTargetCmPer360() const
    {
        const float SafeDpi = FMath::Max(1.0f, ReferenceMouseDpi);
        const float SafeCm = FMath::Max(1.0f, TargetCmPer360);
        return OverwatchCm360Constant / (SafeDpi * SafeCm);
    }

    void Clamp()
    {
        MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.01f, 100.0f);
        ScopedSensitivityMultiplier = FMath::Clamp(ScopedSensitivityMultiplier, 0.1f, 2.0f);
        VehicleLookSensitivityMultiplier = FMath::Clamp(VehicleLookSensitivityMultiplier, 0.1f, 2.0f);
        FirstPersonFovDegrees = FMath::Clamp(FirstPersonFovDegrees, 80.0f, 120.0f);
        ReferenceMouseDpi = FMath::Clamp(ReferenceMouseDpi, 100.0f, 64000.0f);
        TargetCmPer360 = FMath::Clamp(TargetCmPer360, 1.0f, 500.0f);
    }
};
'@

$Pattern = 'USTRUCT\(BlueprintType\)\s*struct FHeroAimSettings\s*\{.*?\n\};'
$HeroTypesNew = [regex]::Replace($HeroTypes, $Pattern, $NewAimSettings, [System.Text.RegularExpressions.RegexOptions]::Singleline)
if ($HeroTypesNew -eq $HeroTypes) {
    throw "Failed to replace FHeroAimSettings in HeroTypes.h. Check the struct name/layout."
}
Set-Content $HeroTypesPath $HeroTypesNew -Encoding UTF8

# ---------------------------------------------------------------------
# 2) HeroCharacter.cpp: avoid UE InputYawScale/FOV scaling by directly
#    applying control rotation in degrees.
# ---------------------------------------------------------------------
$HeroCharacter = Get-Content $HeroCharacterPath -Raw -Encoding UTF8

if ($HeroCharacter -notmatch '#include "GameFramework/Controller.h"') {
    $HeroCharacter = $HeroCharacter -replace '#include "GameFramework/CharacterMovementComponent.h"', "#include `"GameFramework/CharacterMovementComponent.h`"`r`n#include `"GameFramework/Controller.h`""
}

$NewTurn = @'
void AHeroCharacter::Turn(const float Value)
{
    if (Controller && Value != 0.0f)
    {
        const bool bVehicle = VehicleModeComponent && VehicleModeComponent->GetPlayerMode() == EHeroPlayerMode::Vehicle;
        const bool bScoped = WeaponComponent && WeaponComponent->IsAiming();

        FRotator ControlRotation = Controller->GetControlRotation();
        ControlRotation.Yaw += Value * AimSettings.GetFinalDegreesPerCount(bScoped, bVehicle);
        Controller->SetControlRotation(ControlRotation);
    }
}
'@

$NewLookUp = @'
void AHeroCharacter::LookUp(const float Value)
{
    if (Controller && Value != 0.0f)
    {
        const bool bVehicle = VehicleModeComponent && VehicleModeComponent->GetPlayerMode() == EHeroPlayerMode::Vehicle;
        const bool bScoped = WeaponComponent && WeaponComponent->IsAiming();
        const float Invert = AimSettings.bInvertMouseY ? -1.0f : 1.0f;

        FRotator ControlRotation = Controller->GetControlRotation();
        float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
        NormalizedPitch = FMath::Clamp(
            NormalizedPitch + Value * Invert * AimSettings.GetFinalDegreesPerCount(bScoped, bVehicle),
            -89.0f,
            89.0f);

        ControlRotation.Pitch = NormalizedPitch;
        Controller->SetControlRotation(ControlRotation);
    }
}
'@

$TurnPattern = 'void AHeroCharacter::Turn\(const float Value\)\s*\{.*?\n\}'
$LookPattern = 'void AHeroCharacter::LookUp\(const float Value\)\s*\{.*?\n\}'

$HeroCharacterNew = [regex]::Replace($HeroCharacter, $TurnPattern, $NewTurn, [System.Text.RegularExpressions.RegexOptions]::Singleline)
$HeroCharacterNew2 = [regex]::Replace($HeroCharacterNew, $LookPattern, $NewLookUp, [System.Text.RegularExpressions.RegexOptions]::Singleline)

if ($HeroCharacterNew2 -eq $HeroCharacter) {
    throw "Failed to patch Turn/LookUp in HeroCharacter.cpp. Check function names."
}
Set-Content $HeroCharacterPath $HeroCharacterNew2 -Encoding UTF8

# ---------------------------------------------------------------------
# 3) DefaultInput.ini: remove extra 0.07 scaling and FOV scaling.
# ---------------------------------------------------------------------
$Input = Get-Content $InputPath -Raw -Encoding UTF8

$Input = $Input -replace 'AxisKeyName="MouseX",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="MouseX",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="MouseY",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="MouseY",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="Mouse2D",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="MouseX",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="MouseX",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'
$Input = $Input -replace 'AxisKeyName="MouseY",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="MouseY",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'
$Input = $Input -replace 'AxisKeyName="Mouse2D",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'
$Input = $Input -replace 'bEnableFOVScaling=True', 'bEnableFOVScaling=False'
$Input = $Input -replace 'bEnableLegacyInputScales=True', 'bEnableLegacyInputScales=False'

Set-Content $InputPath $Input -Encoding UTF8

# ---------------------------------------------------------------------
# 4) Settings text/range update, best-effort.
# ---------------------------------------------------------------------
if (Test-Path $SettingsWidgetPath) {
    $SettingsWidget = Get-Content $SettingsWidgetPath -Raw -Encoding UTF8
    $SettingsWidget = $SettingsWidget -replace 'Default: 1600 DPI / Sens 1\.0 / 86\.6 cm per 360', 'Overwatch-style: yaw 0.0066, cm/360 = 138600 / (DPI * Sens)'
    $SettingsWidget = $SettingsWidget -replace 'Default: 1600 DPI / Sens 1\.0', 'Overwatch-style: yaw 0.0066'
    $SettingsWidget = $SettingsWidget -replace 'SNew\(SSpinBox<float>\)\.MinValue\(0\.1f\)\.MaxValue\(20\.0f\)\.Value\(WorkingAimSettings\.MouseSensitivity\)', 'SNew(SSpinBox<float>).MinValue(0.01f).MaxValue(100.0f).Value(WorkingAimSettings.MouseSensitivity)'
    Set-Content $SettingsWidgetPath $SettingsWidget -Encoding UTF8
}

Write-Host "Applied Overwatch yaw 0.0066 sensitivity patch."
Write-Host "Runtime: degrees/count = sensitivity * 0.0066"
Write-Host "Display: cm/360 = 138600 / (DPI * sensitivity)"
Write-Host ""

$BuildBat = Join-Path $UERoot "Engine\Build\BatchFiles\Build.bat"
if (!(Test-Path $BuildBat)) {
    Write-Host "Build.bat not found at $BuildBat. Patch applied but build skipped."
    exit 0
}

if (Test-Path (Join-Path $ProjectRoot "Intermediate")) {
    Remove-Item (Join-Path $ProjectRoot "Intermediate") -Recurse -Force
}
if (Test-Path (Join-Path $ProjectRoot "Binaries")) {
    Remove-Item (Join-Path $ProjectRoot "Binaries") -Recurse -Force
}

& $BuildBat HeroVehicleSandboxEditor Win64 Development -Project="$ProjectPath" -WaitMutex -architecture=x64 *> $LogPath
Get-Content $LogPath

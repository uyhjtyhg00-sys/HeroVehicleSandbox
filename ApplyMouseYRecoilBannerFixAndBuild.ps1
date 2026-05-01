
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SourceRoot = Join-Path $ProjectRoot "Source\HeroVehicleSandbox"
$HeroHeader = Join-Path $SourceRoot "Public\HeroCharacter.h"
$HeroCpp = Join-Path $SourceRoot "Private\HeroCharacter.cpp"
$WeaponCpp = Join-Path $SourceRoot "Private\HeroWeaponComponent.cpp"
$HUDCpp = Join-Path $SourceRoot "Private\HeroHUDWidget.cpp"
$InputIni = Join-Path $ProjectRoot "Config\DefaultInput.ini"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

foreach ($Path in @($HeroHeader, $HeroCpp, $InputIni)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }

    $Backup = "$Path.mouse_y_recoil_banner_fix.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

foreach ($Path in @($WeaponCpp, $HUDCpp)) {
    if (Test-Path $Path) {
        $Backup = "$Path.mouse_y_recoil_banner_fix.bak"
        if (!(Test-Path $Backup)) {
            Copy-Item $Path $Backup
        }
    }
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

# ------------------------------------------------------------
# 1) Config: non-inverted MouseY at config level.
#    Gameplay inversion must be handled only by AimSettings.bInvertMouseY.
# ------------------------------------------------------------
$Input = Get-Content $InputIni -Raw -Encoding UTF8

# Keep MouseY axis scale positive. A negative MouseY scale plus bInvertMouseY double-inverts.
$Input = $Input -replace 'AxisMappings=\(AxisName="LookUp",Scale=-1\.000000,Key=MouseY\)', 'AxisMappings=(AxisName="LookUp",Scale=1.000000,Key=MouseY)'
$Input = $Input -replace 'AxisMappings=\(AxisName="LookUp",Key=MouseY,Scale=-1\.000000\)', 'AxisMappings=(AxisName="LookUp",Key=MouseY,Scale=1.000000)'
$Input = $Input -replace 'AxisName="LookUp",Scale=-1\.000000,Key=MouseY', 'AxisName="LookUp",Scale=1.000000,Key=MouseY'
$Input = $Input -replace 'AxisName="LookUp",Key=MouseY,Scale=-1\.000000', 'AxisName="LookUp",Key=MouseY,Scale=1.000000'

# Keep raw mouse axis sensitivity at 1.0.
$Input = $Input -replace 'AxisKeyName="MouseY",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="MouseY",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="MouseY",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="MouseY",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'

Write-Utf8NoBom $InputIni $Input

# ------------------------------------------------------------
# 2) HeroCharacter.h: add ApplyViewKick().
#    This is for recoil/camera impulses and is independent from mouse inversion.
# ------------------------------------------------------------
$Header = Get-Content $HeroHeader -Raw -Encoding UTF8

if ($Header -notmatch 'ApplyViewKick') {
    if ($Header -match 'void ApplyAimSettings\(const FHeroAimSettings& InAimSettings\);') {
        $Header = $Header -replace 'void ApplyAimSettings\(const FHeroAimSettings& InAimSettings\);',
            "void ApplyAimSettings(const FHeroAimSettings& InAimSettings);`r`n`r`n    UFUNCTION(BlueprintCallable, Category=`"Hero|Camera`")`r`n    void ApplyViewKick(float PitchKickDegrees, float YawKickDegrees = 0.0f);"
    }
    elseif ($Header -match 'public:\s*') {
        $Header = $Header -replace 'public:\s*',
            "public:`r`n    UFUNCTION(BlueprintCallable, Category=`"Hero|Camera`")`r`n    void ApplyViewKick(float PitchKickDegrees, float YawKickDegrees = 0.0f);`r`n"
    }
    else {
        throw "Could not insert ApplyViewKick into HeroCharacter.h"
    }
}

Write-Utf8NoBom $HeroHeader $Header

# ------------------------------------------------------------
# 3) HeroCharacter.cpp: fix LookUp sign and add ApplyViewKick.
#    Convention:
#      - Mouse input may come in with UE's raw sign.
#      - We force non-inverted gameplay to feel normal.
#      - Invert option only affects human look input.
#      - Recoil never reads bInvertMouseY.
# ------------------------------------------------------------
$Cpp = Get-Content $HeroCpp -Raw -Encoding UTF8

if ($Cpp -notmatch '#include "GameFramework/Controller.h"') {
    $Cpp = $Cpp -replace '#include "GameFramework/CharacterMovementComponent.h"', "#include `"GameFramework/CharacterMovementComponent.h`"`r`n#include `"GameFramework/Controller.h`""
}

$LookUpFix = @'
void AHeroCharacter::LookUp(const float Value)
{
    if (Controller && Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
    {
        const bool bVehicle = VehicleModeComponent && VehicleModeComponent->GetPlayerMode() == EHeroPlayerMode::Vehicle;
        const bool bScoped = WeaponComponent && WeaponComponent->IsAiming();

        // UE mouse Y/raw axis and FPS pitch convention are opposite in this project.
        // Normal mode: mouse up must look up.
        // Invert option only affects manual look input, not recoil/camera kick.
        constexpr float ProjectMouseYSign = -1.0f;
        const float UserInvertSign = AimSettings.bInvertMouseY ? -1.0f : 1.0f;

        FRotator ControlRotation = Controller->GetControlRotation();
        float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
        NormalizedPitch = FMath::Clamp(
            NormalizedPitch + Value * ProjectMouseYSign * UserInvertSign * AimSettings.GetFinalDegreesPerCount(bScoped, bVehicle),
            -89.0f,
            89.0f);

        ControlRotation.Pitch = NormalizedPitch;
        Controller->SetControlRotation(ControlRotation);
    }
}
'@

if ($Cpp -match 'void AHeroCharacter::LookUp\(const float Value\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::LookUp\(const float Value\)\s*\{.*?\n\}', $LookUpFix, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
else {
    $Cpp = $Cpp.TrimEnd() + "`r`n`r`n" + $LookUpFix
}

$ViewKick = @'
void AHeroCharacter::ApplyViewKick(const float PitchKickDegrees, const float YawKickDegrees)
{
    if (!Controller || (HealthComponent && HealthComponent->IsDead()))
    {
        return;
    }

    // Recoil/camera impulse must be physical and must NOT depend on bInvertMouseY.
    // Positive PitchKickDegrees means "kick camera upward".
    FRotator ControlRotation = Controller->GetControlRotation();
    float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);

    NormalizedPitch = FMath::Clamp(
        NormalizedPitch - FMath::Abs(PitchKickDegrees),
        -89.0f,
        89.0f);

    ControlRotation.Pitch = NormalizedPitch;
    ControlRotation.Yaw += YawKickDegrees;
    Controller->SetControlRotation(ControlRotation);
}
'@

if ($Cpp -match 'void AHeroCharacter::ApplyViewKick\(') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::ApplyViewKick\(.*?\)\s*\{.*?\n\}', $ViewKick, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
else {
    $Cpp = $Cpp.TrimEnd() + "`r`n`r`n" + $ViewKick
}

Write-Utf8NoBom $HeroCpp $Cpp

# ------------------------------------------------------------
# 4) HeroWeaponComponent.cpp: best-effort migration of recoil code.
#    If existing recoil uses AddControllerPitchInput, route it through ApplyViewKick
#    so invert mouse never flips recoil direction.
# ------------------------------------------------------------
if (Test-Path $WeaponCpp) {
    $Weapon = Get-Content $WeaponCpp -Raw -Encoding UTF8

    if ($Weapon -notmatch '#include "HeroCharacter.h"') {
        $Weapon = $Weapon -replace '#include "HeroWeaponComponent.h"', "#include `"HeroWeaponComponent.h`"`r`n#include `"HeroCharacter.h`""
    }

    # Common object names used in generated/handwritten code.
    $Weapon = $Weapon -replace '([A-Za-z_][A-Za-z0-9_]*Hero[A-Za-z0-9_]*|Hero|OwnerHero|HeroCharacter|Character|OwnerCharacter)->AddControllerPitchInput\(-?([A-Za-z_][A-Za-z0-9_]*Recoil[A-Za-z0-9_]*|RecoilPitch[A-Za-z0-9_]*|PitchRecoil[A-Za-z0-9_]*)\);',
                              '$1->ApplyViewKick(FMath::Abs($2), 0.0f);'

    # If the component was casting owner and then using AddControllerPitchInput directly on AHeroCharacter variable with any expression.
    $Weapon = $Weapon -replace '->AddControllerPitchInput\(-FMath::Abs\(([^;\)]+)\)\);', '->ApplyViewKick(FMath::Abs($1), 0.0f);'
    $Weapon = $Weapon -replace '->AddControllerPitchInput\(FMath::Abs\(([^;\)]+)\)\);', '->ApplyViewKick(FMath::Abs($1), 0.0f);'

    Write-Utf8NoBom $WeaponCpp $Weapon
}

# ------------------------------------------------------------
# 5) HeroHUDWidget.cpp: hide the annoying top debug status banner by default.
#    Also remove known status strings if they are hard-coded.
# ------------------------------------------------------------
if (Test-Path $HUDCpp) {
    $Hud = Get-Content $HUDCpp -Raw -Encoding UTF8

    # Make obvious hard-coded status banner empty.
    $Hud = $Hud -replace 'TEXT\("SANDBOX \| NO TEAM \| READY \| 0%"\)', 'TEXT("")'
    $Hud = $Hud -replace 'TEXT\("SANDBOX"\)', 'TEXT("")'
    $Hud = $Hud -replace 'TEXT\("NO TEAM"\)', 'TEXT("")'
    $Hud = $Hud -replace 'TEXT\("READY"\)', 'TEXT("")'

    # If there is a named Status/TopBanner visibility block, collapse it best-effort.
    $Hud = $Hud -replace '\.Visibility\(EVisibility::Visible\)', '.Visibility(EVisibility::HitTestInvisible)'

    Write-Utf8NoBom $HUDCpp $Hud
}

Write-Host "Applied mouse Y / recoil / banner fix."
Write-Host "  - Non-inverted mouse Y now uses project sign correction."
Write-Host "  - Mouse inversion affects manual look only."
Write-Host "  - Recoil uses ApplyViewKick and ignores bInvertMouseY."
Write-Host "  - Known top debug banner strings are hidden/emptied best-effort."
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

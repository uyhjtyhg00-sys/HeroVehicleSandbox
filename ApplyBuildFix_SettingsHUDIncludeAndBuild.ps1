
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SettingsCpp = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroSettingsWidget.cpp"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

if (!(Test-Path $SettingsCpp)) {
    throw "Required file not found: $SettingsCpp"
}

$Backup = "$SettingsCpp.settings_hud_include_fix.bak"
if (!(Test-Path $Backup)) {
    Copy-Item $SettingsCpp $Backup
}

$Text = Get-Content $SettingsCpp -Raw -Encoding UTF8

# UHeroHUDWidget is only forward-declared through HeroPlayerController.generated.h.
# HeroSettingsWidget.cpp calls IsInViewport() on it, so the full type is required.
if ($Text -notmatch '#include "HeroHUDWidget.h"') {
    $Text = $Text -replace '#include "HeroCharacter.h"', "#include `"HeroCharacter.h`"`r`n#include `"HeroHUDWidget.h`""
}

$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($SettingsCpp, $Text, $Utf8NoBom)

Write-Host "Applied HeroSettingsWidget include fix:"
Write-Host "  Added #include `"HeroHUDWidget.h`""
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

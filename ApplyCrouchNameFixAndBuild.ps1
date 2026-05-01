
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SourceRoot = Join-Path $ProjectRoot "Source\HeroVehicleSandbox"
$HeaderPath = Join-Path $SourceRoot "Public\HeroCharacter.h"
$CppPath = Join-Path $SourceRoot "Private\HeroCharacter.cpp"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

foreach ($Path in @($HeaderPath, $CppPath)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }
    $Backup = "$Path.crouch_name_fix.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

# ACharacter already has a built-in member named CrouchedEyeHeight.
# UHT forbids a derived AHeroCharacter member with the same name.
# Rename the project-specific camera target value everywhere in our hero source.
foreach ($Path in @($HeaderPath, $CppPath)) {
    $Text = Get-Content $Path -Raw -Encoding UTF8
    $Text = $Text -replace '\bCrouchedEyeHeight\b', 'HeroCrouchedEyeHeight'
    Set-Content $Path $Text -Encoding UTF8
}

Write-Host "Applied crouch variable rename:"
Write-Host "  CrouchedEyeHeight -> HeroCrouchedEyeHeight"
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


param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$GameModeCpp = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroGameModeBase.cpp"
$HUDCpp = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroHUDWidget.cpp"
$MenuCpp = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroMainMenuWidget.cpp"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

foreach ($Path in @($GameModeCpp, $HUDCpp, $MenuCpp)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }

    $Backup = "$Path.buildfix_role_overlay.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

# ---------------------------------------------------------------------
# Fix 1: UE AActor already has a member named Role.
# C4458 is treated as error, so local/param names named Role must be renamed.
# ---------------------------------------------------------------------
$GameMode = Get-Content $GameModeCpp -Raw -Encoding UTF8

# Most common generated patch pattern.
$GameMode = $GameMode -replace 'const EHeroBotRole Role =', 'const EHeroBotRole BotRole ='
$GameMode = $GameMode -replace '\bSpawnBot\(Team, Role,', 'SpawnBot(Team, BotRole,'
$GameMode = $GameMode -replace '\bSpawnBot\(EHeroTeam::TeamB, Role,', 'SpawnBot(EHeroTeam::TeamB, BotRole,'

# Function definition parameter.
$GameMode = $GameMode -replace 'void AHeroGameModeBase::SpawnBot\(const EHeroTeam Team, const EHeroBotRole Role, const FVector& Location, const FRotator& Rotation\)', 'void AHeroGameModeBase::SpawnBot(const EHeroTeam Team, const EHeroBotRole BotRole, const FVector& Location, const FRotator& Rotation)'

# Any uses inside SpawnBot body that likely refer to the parameter.
$GameMode = $GameMode -replace '\bSetBotRole\(Role\)', 'SetBotRole(BotRole)'
$GameMode = $GameMode -replace '\.BotRole = Role\b', '.BotRole = BotRole'
$GameMode = $GameMode -replace '\bBot->InitializeBot\(Team, Role\)', 'Bot->InitializeBot(Team, BotRole)'
$GameMode = $GameMode -replace '\bBot->SetTeamAndRole\(Team, Role\)', 'Bot->SetTeamAndRole(Team, BotRole)'

Set-Content $GameModeCpp $GameMode -Encoding UTF8

# ---------------------------------------------------------------------
# Fix 2: SOverlay include path.
# In UE Slate, SOverlay is included as Widgets/SOverlay.h, not Widgets/Layout/SOverlay.h.
# ---------------------------------------------------------------------
foreach ($Path in @($HUDCpp, $MenuCpp)) {
    $Text = Get-Content $Path -Raw -Encoding UTF8
    $Text = $Text -replace '#include "Widgets/Layout/SOverlay.h"', '#include "Widgets/SOverlay.h"'
    Set-Content $Path $Text -Encoding UTF8
}

Write-Host "Applied build fixes:"
Write-Host "  1. HeroGameModeBase.cpp: Role -> BotRole"
Write-Host "  2. HeroHUDWidget.cpp / HeroMainMenuWidget.cpp: Widgets/Layout/SOverlay.h -> Widgets/SOverlay.h"
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

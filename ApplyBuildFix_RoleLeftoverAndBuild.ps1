
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$HeaderPath = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Public\HeroGameModeBase.h"
$CppPath = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroGameModeBase.cpp"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

foreach ($Path in @($HeaderPath, $CppPath)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }

    $Backup = "$Path.role_leftover_fix.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

# Header declaration: avoid parameter name Role because AActor already has private member Role.
$Header = Get-Content $HeaderPath -Raw -Encoding UTF8
$Header = $Header -replace 'void SpawnBot\(EHeroTeam Team, EHeroBotRole Role, const FVector& Location, const FRotator& Rotation\);',
                           'void SpawnBot(EHeroTeam Team, EHeroBotRole BotRole, const FVector& Location, const FRotator& Rotation);'
Set-Content $HeaderPath $Header -Encoding UTF8

# Source: finish any leftover Role references from the previous partial rename.
$Cpp = Get-Content $CppPath -Raw -Encoding UTF8

# Rename remaining local declarations.
$Cpp = $Cpp -replace 'const EHeroBotRole Role =', 'const EHeroBotRole BotRole ='

# Rename all SpawnBot call arguments where the second arg was accidentally left as AActor::Role.
$Cpp = $Cpp -replace 'SpawnBot\(EHeroTeam::TeamA, Role,', 'SpawnBot(EHeroTeam::TeamA, BotRole,'
$Cpp = $Cpp -replace 'SpawnBot\(EHeroTeam::TeamB, Role,', 'SpawnBot(EHeroTeam::TeamB, BotRole,'
$Cpp = $Cpp -replace 'SpawnBot\(Team, Role,', 'SpawnBot(Team, BotRole,'

# Rename function definition parameter.
$Cpp = $Cpp -replace 'void AHeroGameModeBase::SpawnBot\(const EHeroTeam Team, const EHeroBotRole Role, const FVector& Location, const FRotator& Rotation\)',
                     'void AHeroGameModeBase::SpawnBot(const EHeroTeam Team, const EHeroBotRole BotRole, const FVector& Location, const FRotator& Rotation)'

# Rename likely parameter uses inside the function body.
$Cpp = $Cpp -replace '\bSetBotRole\(Role\)', 'SetBotRole(BotRole)'
$Cpp = $Cpp -replace '\bInitializeBot\(Team, Role\)', 'InitializeBot(Team, BotRole)'
$Cpp = $Cpp -replace '\bSetTeamAndRole\(Team, Role\)', 'SetTeamAndRole(Team, BotRole)'
$Cpp = $Cpp -replace '\bBotRole = Role\b', 'BotRole = BotRole'

Set-Content $CppPath $Cpp -Encoding UTF8

Write-Host "Applied leftover Role fix."
Write-Host "Fixed:"
Write-Host "  SpawnBot(..., Role, ...) -> SpawnBot(..., BotRole, ...)"
Write-Host "  SpawnBot declaration parameter Role -> BotRole"
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

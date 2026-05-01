
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$HeroCpp = Join-Path $ProjectRoot "Source\HeroVehicleSandbox\Private\HeroCharacter.cpp"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor_FastFallJump_v2.log"

if (!(Test-Path $HeroCpp)) {
    throw "Required file not found: $HeroCpp"
}

$Backup = "$HeroCpp.fast_fall_jump_tuning_v2.bak"
if (!(Test-Path $Backup)) {
    Copy-Item $HeroCpp $Backup
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

$Text = Get-Content $HeroCpp -Raw -Encoding UTF8

# Fast fall / jump tuning. Keep comments ASCII-only to avoid encoding/regex issues.
$Text = $Text -replace 'Movement->JumpZVelocity\s*=\s*[0-9.]+f\s*;', 'Movement->JumpZVelocity = 585.0f;'
$Text = $Text -replace 'Movement->AirControl\s*=\s*[0-9.]+f\s*;', 'Movement->AirControl = 0.42f;'
$Text = $Text -replace 'Movement->AirControlBoostMultiplier\s*=\s*[0-9.]+f\s*;', 'Movement->AirControlBoostMultiplier = 0.65f;'
$Text = $Text -replace 'Movement->AirControlBoostVelocityThreshold\s*=\s*[0-9.]+f\s*;', 'Movement->AirControlBoostVelocityThreshold = 35.0f;'
$Text = $Text -replace 'Movement->FallingLateralFriction\s*=\s*[0-9.]+f\s*;', 'Movement->FallingLateralFriction = 0.12f;'
$Text = $Text -replace 'Movement->BrakingDecelerationFalling\s*=\s*[0-9.]+f\s*;', 'Movement->BrakingDecelerationFalling = 120.0f;'

if ($Text -match 'Movement->GravityScale\s*=') {
    $Text = $Text -replace 'Movement->GravityScale\s*=\s*[0-9.]+f\s*;', 'Movement->GravityScale = 2.25f;'
}
elseif ($Text -match 'Movement->JumpZVelocity = 585\.0f;') {
    $Text = $Text -replace 'Movement->JumpZVelocity = 585\.0f;', "Movement->JumpZVelocity = 585.0f;`r`n    Movement->GravityScale = 2.25f;"
}
else {
    throw "Could not find JumpZVelocity assignment to insert GravityScale."
}

if ($Text -notmatch 'Fast-fall tuning') {
    $Text = $Text -replace 'Movement->JumpZVelocity = 585\.0f;', "// Function: Fast-fall tuning keeps jump responsive but reduces floaty hang time.`r`n    Movement->JumpZVelocity = 585.0f;"
}

Write-Utf8NoBom $HeroCpp $Text

Write-Host "Applied fast fall / jump tuning v2."
Write-Host "  JumpZVelocity = 585.0"
Write-Host "  GravityScale = 2.25"
Write-Host "  AirControl = 0.42"
Write-Host "  FallingLateralFriction = 0.12"
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

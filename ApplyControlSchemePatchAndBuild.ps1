
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SourceRoot = Join-Path $ProjectRoot "Source\HeroVehicleSandbox"
$HeroHeader = Join-Path $SourceRoot "Public\HeroCharacter.h"
$HeroCpp = Join-Path $SourceRoot "Private\HeroCharacter.cpp"
$VehicleCpp = Join-Path $SourceRoot "Private\HeroDriveableVehiclePawn.cpp"
$InputIni = Join-Path $ProjectRoot "Config\DefaultInput.ini"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor_ControlScheme.log"

foreach ($Path in @($HeroHeader, $HeroCpp, $InputIni)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }

    $Backup = "$Path.control_scheme_patch.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

if (Test-Path $VehicleCpp) {
    $Backup = "$VehicleCpp.control_scheme_patch.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $VehicleCpp $Backup
    }
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

# ------------------------------------------------------------
# 1) Input scheme
#    G       = Interact / enter / exit vehicle
#    F       = Melee
#    Ctrl+S  = Dev vehicle spawn
#    Q/E no longer perform vehicle mode.
# ------------------------------------------------------------
$Input = Get-Content $InputIni -Raw -Encoding UTF8

# Remove old mappings for these actions to avoid duplicate/legacy controls.
$Lines = $Input -split "`r?`n"
$Filtered = New-Object System.Collections.Generic.List[string]
foreach ($Line in $Lines) {
    if ($Line -match 'ActionName="ToggleVehicleMode"') { continue }
    if ($Line -match 'ActionName="Interact"') { continue }
    if ($Line -match 'ActionName="Melee"') { continue }
    if ($Line -match 'ActionName="DevSpawnVehicle"') { continue }
    $Filtered.Add($Line)
}
$Input = ($Filtered -join "`r`n").TrimEnd()

$Input += "`r`n+ActionMappings=(ActionName=`"Interact`",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=G)"
$Input += "`r`n+ActionMappings=(ActionName=`"Melee`",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=F)"
$Input += "`r`n+ActionMappings=(ActionName=`"DevSpawnVehicle`",bShift=False,bCtrl=True,bAlt=False,bCmd=False,Key=S)`r`n"

Write-Utf8NoBom $InputIni $Input

# ------------------------------------------------------------
# 2) HeroCharacter.h declarations
# ------------------------------------------------------------
$Header = Get-Content $HeroHeader -Raw -Encoding UTF8

if ($Header -notmatch 'MeleeAttack') {
    if ($Header -match 'void Interact\(\);') {
        $Header = $Header -replace 'void Interact\(\);',
            "void Interact();`r`n    // Function: Perform a short-range hero melee attack. Bound to F.`r`n    void MeleeAttack();`r`n    // Function: Development-only vehicle spawn. Bound to Ctrl+S.`r`n    void DevSpawnVehicleForTesting();"
    }
    else {
        $Header = $Header -replace 'void ToggleVehicleMode\(\);',
            "void ToggleVehicleMode();`r`n    void Interact();`r`n    void MeleeAttack();`r`n    void DevSpawnVehicleForTesting();"
    }
}

Write-Utf8NoBom $HeroHeader $Header

# ------------------------------------------------------------
# 3) HeroCharacter.cpp controls and behavior
# ------------------------------------------------------------
$Cpp = Get-Content $HeroCpp -Raw -Encoding UTF8

foreach ($Inc in @(
    '#include "HeroDriveableVehiclePawn.h"',
    '#include "EngineUtils.h"'
)) {
    if ($Cpp -notmatch [regex]::Escape($Inc)) {
        $Cpp = $Cpp -replace '#include "HeroCharacter.h"', "#include `"HeroCharacter.h`"`r`n$Inc"
    }
}

$NewSetup = @'
void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Function: Bind hero movement, combat, interact, melee, and dev-only vehicle spawn.
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHeroCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHeroCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AHeroCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AHeroCharacter::LookUp);

    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AHeroCharacter::StartHeroCrouch);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &AHeroCharacter::StopHeroCrouch);

    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AHeroCharacter::StartFire);
    PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AHeroCharacter::StopFire);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &AHeroCharacter::StartAim);
    PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &AHeroCharacter::StopAim);
    PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AHeroCharacter::Reload);

    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AHeroCharacter::Interact);
    PlayerInputComponent->BindAction(TEXT("Melee"), IE_Pressed, this, &AHeroCharacter::MeleeAttack);
    PlayerInputComponent->BindAction(TEXT("DevSpawnVehicle"), IE_Pressed, this, &AHeroCharacter::DevSpawnVehicleForTesting);
}
'@

if ($Cpp -match 'void AHeroCharacter::SetupPlayerInputComponent\(UInputComponent\* PlayerInputComponent\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::SetupPlayerInputComponent\(UInputComponent\* PlayerInputComponent\)\s*\{.*?\n\}', $NewSetup, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}

$Toggle = @'
void AHeroCharacter::ToggleVehicleMode()
{
    // Function: Legacy vehicle-mode input path kept harmless. Real interaction is on G.
    Interact();
}
'@
if ($Cpp -match 'void AHeroCharacter::ToggleVehicleMode\(\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::ToggleVehicleMode\(\)\s*\{.*?\n\}', $Toggle, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
else {
    $Cpp += "`r`n`r`n" + $Toggle
}

$Interact = @'
void AHeroCharacter::Interact()
{
    // Function: G is the single gameplay interaction key. It enters the nearest real vehicle only.
    EnterNearestOrSpawnedVehicle();
}
'@
if ($Cpp -match 'void AHeroCharacter::Interact\(\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::Interact\(\)\s*\{.*?\n\}', $Interact, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
else {
    $Cpp += "`r`n`r`n" + $Interact
}

$Enter = @'
void AHeroCharacter::EnterNearestOrSpawnedVehicle()
{
    if (HealthComponent && HealthComponent->IsDead())
    {
        return;
    }

    AController* CurrentController = Controller;
    if (!CurrentController)
    {
        return;
    }

    // Function: Interaction may only enter an existing nearby vehicle.
    // It must not spawn a new car by itself.
    AHeroDriveableVehiclePawn* Vehicle = FindNearestDriveableVehicle();
    if (Vehicle)
    {
        Vehicle->EnterVehicle(this, CurrentController);
    }
}
'@
if ($Cpp -match 'void AHeroCharacter::EnterNearestOrSpawnedVehicle\(\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::EnterNearestOrSpawnedVehicle\(\)\s*\{.*?\n\}', $Enter, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}

$Melee = @'
void AHeroCharacter::MeleeAttack()
{
    if (!Controller || (HealthComponent && HealthComponent->IsDead()))
    {
        return;
    }

    // Function: F performs a short-range forward trace and damages the first health component hit.
    constexpr float MeleeRangeCm = 190.0f;
    constexpr float MeleeDamage = 40.0f;

    const FVector Start = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : GetActorLocation() + FVector(0.0f, 0.0f, 64.0f);
    const FVector Direction = FirstPersonCamera ? FirstPersonCamera->GetForwardVector() : GetActorForwardVector();
    const FVector End = Start + Direction * MeleeRangeCm;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(HeroMeleeAttack), false, this);
    FHitResult Hit;

    if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (UHeroHealthComponent* HitHealth = HitActor->FindComponentByClass<UHeroHealthComponent>())
            {
                HitHealth->ApplyDamage(MeleeDamage, this);
            }
        }
    }
}
'@
if ($Cpp -match 'void AHeroCharacter::MeleeAttack\(\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::MeleeAttack\(\)\s*\{.*?\n\}', $Melee, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
else {
    $Cpp += "`r`n`r`n" + $Melee
}

$DevSpawn = @'
void AHeroCharacter::DevSpawnVehicleForTesting()
{
    if (HealthComponent && HealthComponent->IsDead())
    {
        return;
    }

    // Function: Ctrl+S is a development-only helper that spawns a real RaceVehicleCore pawn in front of the hero.
    // It intentionally does not enter the car. Press G near the car to enter.
    SpawnDriveableVehicleForEntry();
}
'@
if ($Cpp -match 'void AHeroCharacter::DevSpawnVehicleForTesting\(\)') {
    $Cpp = [regex]::Replace($Cpp, 'void AHeroCharacter::DevSpawnVehicleForTesting\(\)\s*\{.*?\n\}', $DevSpawn, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
else {
    $Cpp += "`r`n`r`n" + $DevSpawn
}

Write-Utf8NoBom $HeroCpp $Cpp

# ------------------------------------------------------------
# 4) Vehicle pawn: G exits vehicle. Q/E no longer needed.
# ------------------------------------------------------------
if (Test-Path $VehicleCpp) {
    $Vehicle = Get-Content $VehicleCpp -Raw -Encoding UTF8

    $VehicleSetup = @'
void AHeroDriveableVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Function: Bind real vehicle drive controls and the shared G interaction key for exit.
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHeroDriveableVehiclePawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHeroDriveableVehiclePawn::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AHeroDriveableVehiclePawn::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AHeroDriveableVehiclePawn::LookUp);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AHeroDriveableVehiclePawn::StartHandbrake);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AHeroDriveableVehiclePawn::StopHandbrake);
    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AHeroDriveableVehiclePawn::ExitVehicle);
}
'@

    if ($Vehicle -match 'void AHeroDriveableVehiclePawn::SetupPlayerInputComponent\(UInputComponent\* PlayerInputComponent\)') {
        $Vehicle = [regex]::Replace($Vehicle, 'void AHeroDriveableVehiclePawn::SetupPlayerInputComponent\(UInputComponent\* PlayerInputComponent\)\s*\{.*?\n\}', $VehicleSetup, [System.Text.RegularExpressions.RegexOptions]::Singleline)
    }

    Write-Utf8NoBom $VehicleCpp $Vehicle
}

Write-Host "Applied control scheme patch."
Write-Host "  G       = Interact / enter / exit vehicle"
Write-Host "  F       = Melee attack"
Write-Host "  Ctrl+S  = Development vehicle spawn"
Write-Host "  Q/E no longer trigger vehicle mode or vehicle spawn"
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

& $BuildBat HeroVehicleSandboxEditor Win64 Development -Project="$ProjectPath" -WaitMutex -architecture=x64 *> (Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor_ControlScheme.log")
Get-Content (Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor_ControlScheme.log")

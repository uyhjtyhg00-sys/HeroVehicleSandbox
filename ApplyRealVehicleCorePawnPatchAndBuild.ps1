param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SourceRoot = Join-Path $ProjectRoot "Source\HeroVehicleSandbox"
$PublicRoot = Join-Path $SourceRoot "Public"
$PrivateRoot = Join-Path $SourceRoot "Private"
$ConfigRoot = Join-Path $ProjectRoot "Config"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor_RealVehicle.log"

$HeroCharacterH = Join-Path $PublicRoot "HeroCharacter.h"
$HeroCharacterCpp = Join-Path $PrivateRoot "HeroCharacter.cpp"
$HeroPlayerControllerH = Join-Path $PublicRoot "HeroPlayerController.h"
$HeroPlayerControllerCpp = Join-Path $PrivateRoot "HeroPlayerController.cpp"
$RaceVehicleCoreCpp = Join-Path $PrivateRoot "LegacyVehicle\RaceVehicleCore.cpp"
$InputIni = Join-Path $ConfigRoot "DefaultInput.ini"
$DocsDir = Join-Path $ProjectRoot "Docs"

foreach ($Path in @($HeroCharacterH, $HeroCharacterCpp, $HeroPlayerControllerH, $HeroPlayerControllerCpp, $RaceVehicleCoreCpp, $InputIni)) {
    if (!(Test-Path $Path)) { throw "Required file not found: $Path" }
    $Backup = "$Path.real_vehicle_core_patch.bak"
    if (!(Test-Path $Backup)) { Copy-Item $Path $Backup }
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

# -------------------------------------------------------------------
# 1. Copy the new real vehicle Pawn source files.
# -------------------------------------------------------------------
Copy-Item (Join-Path $PSScriptRoot "Source\HeroVehicleSandbox\Public\HeroDriveableVehiclePawn.h") (Join-Path $PublicRoot "HeroDriveableVehiclePawn.h") -Force
Copy-Item (Join-Path $PSScriptRoot "Source\HeroVehicleSandbox\Private\HeroDriveableVehiclePawn.cpp") (Join-Path $PrivateRoot "HeroDriveableVehiclePawn.cpp") -Force

# -------------------------------------------------------------------
# 2. HeroCharacter.h: declare vehicle entry helpers and tuning values.
# -------------------------------------------------------------------
$H = Get-Content $HeroCharacterH -Raw -Encoding UTF8
if ($H -notmatch 'class AHeroDriveableVehiclePawn;') {
    $H = $H -replace 'class UHeroTeamComponent;', "class UHeroTeamComponent;`r`nclass AHeroDriveableVehiclePawn;"
}
if ($H -notmatch 'FindNearestDriveableVehicle') {
    $H = $H -replace 'void Interact\(\);', "void Interact();`r`n    // 기능: 주변에 이미 스폰된 운전 가능 차량이 있는지 탐색한다.`r`n    AHeroDriveableVehiclePawn* FindNearestDriveableVehicle() const;`r`n    // 기능: 주변 차량이 없을 때 Hero 앞쪽에 원조 RaceVehicleCore 기반 차량을 스폰한다.`r`n    AHeroDriveableVehiclePawn* SpawnDriveableVehicleForEntry();`r`n    // 기능: 찾거나 스폰한 실제 차량 Pawn에 탑승한다.`r`n    void EnterNearestOrSpawnedVehicle();"
}
if ($H -notmatch 'DriveableVehicleSearchRadiusCm') {
    $Insert = @'

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle Entry", meta=(AllowPrivateAccess="true"))
    float DriveableVehicleSearchRadiusCm = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle Entry", meta=(AllowPrivateAccess="true"))
    float DriveableVehicleSpawnDistanceCm = 280.0f;
'@
    $H = $H -replace 'UPROPERTY\(EditAnywhere, BlueprintReadWrite, Category="Hero\|Settings", meta=\(AllowPrivateAccess="true"\)\)\s*FHeroAimSettings AimSettings;', ($Insert + "`r`n    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=`"Hero|Settings`", meta=(AllowPrivateAccess=`"true`"))`r`n    FHeroAimSettings AimSettings;")
}
Write-Utf8NoBom $HeroCharacterH $H

# -------------------------------------------------------------------
# 3. HeroCharacter.cpp: replace fake speed mode with real vehicle entry.
# -------------------------------------------------------------------
$C = Get-Content $HeroCharacterCpp -Raw -Encoding UTF8
if ($C -notmatch '#include "HeroDriveableVehiclePawn.h"') {
    $C = $C -replace '#include "HeroCharacter.h"', "#include `"HeroCharacter.h`"`r`n`r`n#include `"EngineUtils.h`"`r`n#include `"HeroDriveableVehiclePawn.h`""
}
$Toggle = @'
void AHeroCharacter::ToggleVehicleMode()
{
    // 기능: 기존의 속도 버프식 Vehicle Mode를 쓰지 않고, 실제 원조 RaceVehicleCore 차량 Pawn에 탑승한다.
    EnterNearestOrSpawnedVehicle();
}
'@
$Interact = @'
void AHeroCharacter::Interact()
{
    // 기능: E 상호작용으로 주변 차량에 탑승하거나 없으면 새 차량을 스폰해 탑승한다.
    EnterNearestOrSpawnedVehicle();
}
'@
$C = [regex]::Replace($C, 'void AHeroCharacter::ToggleVehicleMode\(\)\s*\{.*?\n\}', $Toggle, [System.Text.RegularExpressions.RegexOptions]::Singleline)
$C = [regex]::Replace($C, 'void AHeroCharacter::Interact\(\)\s*\{.*?\n\}', $Interact, [System.Text.RegularExpressions.RegexOptions]::Singleline)

$ApplyMode = @'
void AHeroCharacter::ApplyModeToComponents(const EHeroPlayerMode NewMode)
{
    // 기능: Hero 본체 상태만 갱신한다. 실제 차량 이동은 AHeroDriveableVehiclePawn과 RaceVehicleCore가 담당한다.
    WeaponComponent->SetPlayerMode(NewMode);
    GetCharacterMovement()->MaxWalkSpeed = HumanWalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;
}
'@
$C = [regex]::Replace($C, 'void AHeroCharacter::ApplyModeToComponents\(const EHeroPlayerMode NewMode\)\s*\{.*?\n\}', $ApplyMode, [System.Text.RegularExpressions.RegexOptions]::Singleline)

if ($C -notmatch 'AHeroCharacter::FindNearestDriveableVehicle') {
$VehicleEntryFuncs = @'

AHeroDriveableVehiclePawn* AHeroCharacter::FindNearestDriveableVehicle() const
{
    // 기능: 플레이어 주변에 있는 가장 가까운 실제 차량 Pawn을 찾아 재사용한다.
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AHeroDriveableVehiclePawn* BestVehicle = nullptr;
    float BestDistSq = FMath::Square(DriveableVehicleSearchRadiusCm);
    const FVector HeroLocation = GetActorLocation();

    for (TActorIterator<AHeroDriveableVehiclePawn> It(World); It; ++It)
    {
        AHeroDriveableVehiclePawn* Vehicle = *It;
        if (!Vehicle || Vehicle->IsOccupied())
        {
            continue;
        }

        const float DistSq = FVector::DistSquared(HeroLocation, Vehicle->GetActorLocation());
        if (DistSq <= BestDistSq)
        {
            BestDistSq = DistSq;
            BestVehicle = Vehicle;
        }
    }

    return BestVehicle;
}

AHeroDriveableVehiclePawn* AHeroCharacter::SpawnDriveableVehicleForEntry()
{
    // 기능: 주변 차량이 없을 때 Hero 앞쪽에 원조 RaceVehicleCore 기반 차량을 새로 스폰한다.
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * DriveableVehicleSpawnDistanceCm + FVector(0.0f, 0.0f, 35.0f);
    const FRotator SpawnRotation(0.0f, GetActorRotation().Yaw, 0.0f);

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    return World->SpawnActor<AHeroDriveableVehiclePawn>(AHeroDriveableVehiclePawn::StaticClass(), SpawnLocation, SpawnRotation, Params);
}

void AHeroCharacter::EnterNearestOrSpawnedVehicle()
{
    // 기능: 차량 탑승 요청을 주변 차량 탐색 → 없으면 스폰 → Possess 순서로 처리한다.
    if (HealthComponent && HealthComponent->IsDead())
    {
        return;
    }

    AController* OwningController = GetController();
    if (!OwningController)
    {
        return;
    }

    AHeroDriveableVehiclePawn* Vehicle = FindNearestDriveableVehicle();
    if (!Vehicle)
    {
        Vehicle = SpawnDriveableVehicleForEntry();
    }

    if (Vehicle)
    {
        Vehicle->EnterVehicle(this, OwningController);
    }
}
'@
    $C = $C.TrimEnd() + "`r`n" + $VehicleEntryFuncs + "`r`n"
}
Write-Utf8NoBom $HeroCharacterCpp $C

# -------------------------------------------------------------------
# 4. HeroPlayerController: HUD branch for real vehicle Pawn.
# -------------------------------------------------------------------
$PH = Get-Content $HeroPlayerControllerH -Raw -Encoding UTF8
if ($PH -notmatch 'class AHeroDriveableVehiclePawn;') {
    $PH = $PH -replace 'class AHeroCharacter;', "class AHeroCharacter;`r`nclass AHeroDriveableVehiclePawn;"
}
Write-Utf8NoBom $HeroPlayerControllerH $PH

$PC = Get-Content $HeroPlayerControllerCpp -Raw -Encoding UTF8
if ($PC -notmatch '#include "HeroDriveableVehiclePawn.h"') {
    $PC = $PC -replace '#include "HeroCharacter.h"', "#include `"HeroCharacter.h`"`r`n#include `"HeroDriveableVehiclePawn.h`""
}
$VehicleHudBranch = @'
    if (AHeroDriveableVehiclePawn* VehiclePawn = Cast<AHeroDriveableVehiclePawn>(GetPawn()))
    {
        // 기능: 실제 차량 Pawn 탑승 중에는 HeroCharacter 가짜 속도 대신 RaceVehicleCore 값으로 HUD를 갱신한다.
        HeroHUD->SetPlayerMode(EHeroPlayerMode::Vehicle);
        HeroHUD->SetVehicleSpeed(VehiclePawn->GetSpeedKmh());
        HeroHUD->SetVehicleHealth(VehiclePawn->GetVehicleHealth(), 100.0f);
        HeroHUD->SetWeaponName(FText::FromString(TEXT("Race Core Vehicle")));
        HeroHUD->SetAmmo(VehiclePawn->GetCurrentGear(), FMath::RoundToInt(VehiclePawn->GetVehicleTelemetry().EngineRpm));
        HeroHUD->SetCrosshairVisible(false);
        return;
    }

'@
if ($PC -notmatch 'RaceVehicleCore 값으로 HUD를 갱신') {
    $PC = $PC -replace 'AHeroCharacter\* Hero = Cast<AHeroCharacter>\(GetPawn\(\)\);', ($VehicleHudBranch + '    AHeroCharacter* Hero = Cast<AHeroCharacter>(GetPawn());')
}
Write-Utf8NoBom $HeroPlayerControllerCpp $PC

# -------------------------------------------------------------------
# 5. Legacy RaceVehicleCore ResetState: respect explicit Z for spawned vehicles.
# -------------------------------------------------------------------
$RC = Get-Content $RaceVehicleCoreCpp -Raw -Encoding UTF8
$RC = $RC -replace 'const double InitialZ = static_cast<double>\(Parameters\.GroundProbeHeightMeters \+ Parameters\.TargetRideHeightMeters\);', 'const double DefaultZ = static_cast<double>(Parameters.GroundProbeHeightMeters + Parameters.TargetRideHeightMeters);`r`n    // 기능: 외부 Pawn/체크포인트가 명시한 Z가 있으면 존중하고, 없을 때만 기본 지상고를 쓴다.`r`n    const double InitialZ = std::abs(ZMeters) > 0.001 ? ZMeters : DefaultZ;'
Write-Utf8NoBom $RaceVehicleCoreCpp $RC

# -------------------------------------------------------------------
# 6. Config: keep legacy input and make Q/E shared vehicle enter/exit actions.
# -------------------------------------------------------------------
$I = Get-Content $InputIni -Raw -Encoding UTF8
$I = $I -replace 'DefaultPlayerInputClass=.*', 'DefaultPlayerInputClass=/Script/Engine.PlayerInput'
$I = $I -replace 'DefaultInputComponentClass=.*', 'DefaultInputComponentClass=/Script/Engine.InputComponent'
if ($I -notmatch 'ActionName="ToggleVehicleMode"') { $I += "`r`n+ActionMappings=(ActionName=`"ToggleVehicleMode`",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Q)`r`n" }
if ($I -notmatch 'ActionName="Interact"') { $I += "`r`n+ActionMappings=(ActionName=`"Interact`",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=E)`r`n" }
Write-Utf8NoBom $InputIni $I

# -------------------------------------------------------------------
# 7. Docs: comment policy for future patches.
# -------------------------------------------------------------------
if (!(Test-Path $DocsDir)) { New-Item -ItemType Directory -Path $DocsDir | Out-Null }
$Policy = @'
# Function Comment Policy

Every gameplay/UI function added or modified by patches should include a short Korean `// 기능:` comment explaining its purpose.

Required style:

```cpp
// 기능: 이 함수가 플레이어/시스템 관점에서 수행하는 역할을 한 문장으로 설명한다.
void SomeFunction();
```

Reason:

- Keeps future patches consistent.
- Makes large C++ files easier to audit.
- Helps distinguish placeholder code from real gameplay code.
'@
Write-Utf8NoBom (Join-Path $DocsDir "FunctionCommentPolicy.md") $Policy

Write-Host "Applied real vehicle core pawn patch."
Write-Host "Q/E from Hero: find or spawn real AHeroDriveableVehiclePawn and possess it."
Write-Host "Q/E from Vehicle: exit back to Hero."
Write-Host "Vehicle movement now uses LegacyVehicle FRaceVehicleCore."
Write-Host ""

$BuildBat = Join-Path $UERoot "Engine\Build\BatchFiles\Build.bat"
if (!(Test-Path $BuildBat)) { Write-Host "Build.bat not found. Patch applied, build skipped."; exit 0 }

if (Test-Path (Join-Path $ProjectRoot "Intermediate")) { Remove-Item (Join-Path $ProjectRoot "Intermediate") -Recurse -Force }
if (Test-Path (Join-Path $ProjectRoot "Binaries")) { Remove-Item (Join-Path $ProjectRoot "Binaries") -Recurse -Force }

& $BuildBat HeroVehicleSandboxEditor Win64 Development -Project="$ProjectPath" -WaitMutex -architecture=x64 *> $LogPath
Get-Content $LogPath


param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SourceRoot = Join-Path $ProjectRoot "Source\HeroVehicleSandbox"

$WeaponCoreCpp = Join-Path $SourceRoot "Private\HeroWeaponCore.cpp"
$HUDCpp = Join-Path $SourceRoot "Private\HeroHUDWidget.cpp"
$MenuHeader = Join-Path $SourceRoot "Public\HeroMainMenuWidget.h"
$MenuCpp = Join-Path $SourceRoot "Private\HeroMainMenuWidget.cpp"
$HeroHeader = Join-Path $SourceRoot "Public\HeroCharacter.h"
$HeroCpp = Join-Path $SourceRoot "Private\HeroCharacter.cpp"
$VehicleCpp = Join-Path $SourceRoot "Private\HeroDriveableVehiclePawn.cpp"
$PlayerControllerCpp = Join-Path $SourceRoot "Private\HeroPlayerController.cpp"

$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor_AmmoVehicleHUDMenuCrouchFix.log"

$Required = @($WeaponCoreCpp, $HUDCpp, $MenuHeader, $MenuCpp, $HeroHeader, $HeroCpp, $PlayerControllerCpp)
foreach ($Path in $Required) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }
}

foreach ($Path in @($WeaponCoreCpp, $HUDCpp, $MenuHeader, $MenuCpp, $HeroHeader, $HeroCpp, $VehicleCpp, $PlayerControllerCpp)) {
    if (Test-Path $Path) {
        $Backup = "$Path.ammo_vehicle_hud_menu_crouch_fix.bak"
        if (!(Test-Path $Backup)) {
            Copy-Item $Path $Backup
        }
    }
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

# ---------------------------------------------------------------------
# 1) Infinite reserve ammo.
#    Keep magazine/reload gameplay, but reserve should not run out in sandbox.
# ---------------------------------------------------------------------
$WeaponCore = Get-Content $WeaponCoreCpp -Raw -Encoding UTF8

$WeaponCore = [regex]::Replace(
    $WeaponCore,
    'void FHeroWeaponCore::Initialize\(const FHeroWeaponConfig& InConfig\)\s*\{.*?\n\}',
@'
void FHeroWeaponCore::Initialize(const FHeroWeaponConfig& InConfig)
{
    // Function: Initialize weapon state. Reserve ammo is treated as sandbox-infinite.
    Config = InConfig;
    Config.MagazineSize = FMath::Max(1, Config.MagazineSize);
    Config.ReserveAmmo = 999999;
    Config.FireRateRoundsPerMinute = FMath::Max(1.0f, Config.FireRateRoundsPerMinute);
    AmmoInMagazine = Config.MagazineSize;
    ReserveAmmo = Config.ReserveAmmo;
    bReloading = false;
    ReloadRemainingSeconds = 0.0f;
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

$WeaponCore = [regex]::Replace(
    $WeaponCore,
    'bool FHeroWeaponCore::StartReload\(\)\s*\{.*?\n\}',
@'
bool FHeroWeaponCore::StartReload()
{
    // Function: Start reload when the magazine is not full. Reserve ammo is infinite in sandbox.
    if (bReloading || AmmoInMagazine >= Config.MagazineSize)
    {
        return false;
    }

    bReloading = true;
    ReloadRemainingSeconds = FMath::Max(0.01f, Config.ReloadSeconds);
    return true;
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

$WeaponCore = [regex]::Replace(
    $WeaponCore,
    'void FHeroWeaponCore::FinishReload\(\)\s*\{.*?\n\}',
@'
void FHeroWeaponCore::FinishReload()
{
    // Function: Refill magazine without consuming reserve ammo.
    AmmoInMagazine = Config.MagazineSize;
    ReserveAmmo = Config.ReserveAmmo;
    bReloading = false;
    ReloadRemainingSeconds = 0.0f;
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

Write-Utf8NoBom $WeaponCoreCpp $WeaponCore

# ---------------------------------------------------------------------
# 2) HUD cleanup.
#    - No comment-like prompts in HUD.
#    - No objective on pure Sandbox/NoTeam.
#    - Vehicle info must not replace the weapon display area.
#    - Infinite ammo display uses infinity sign.
# ---------------------------------------------------------------------
$HUD = Get-Content $HUDCpp -Raw -Encoding UTF8

# Initial widget texts.
$HUD = $HUD -replace 'TEXT\("Objective inactive"\)', 'TEXT("")'
$HUD = $HUD -replace 'TEXT\("BOTS 0/0"\)', 'TEXT("")'
$HUD = $HUD -replace 'TEXT\("Q: Vehicle Mode \| E: Interact \| Ctrl/C: Crouch"\)', 'TEXT("")'
$HUD = $HUD -replace 'TEXT\("30 / 120"\)', 'TEXT("30 / ∞")'
$HUD = $HUD -replace 'TEXT\("HEAT 0%"\)', 'TEXT("")'

# SetAmmo.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetAmmo\(const int32 AmmoInMagazine, const int32 ReserveAmmo\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetAmmo(const int32 AmmoInMagazine, const int32 ReserveAmmo)
{
    // Function: Show weapon ammo only for human weapon HUD. Reserve ammo is displayed as infinite.
    if (!AmmoText)
    {
        return;
    }

    if (PlayerMode == EHeroPlayerMode::Vehicle || AmmoInMagazine < 0)
    {
        AmmoText->SetText(FText::GetEmpty());
        return;
    }

    const FString ReserveText = ReserveAmmo >= 999000 ? TEXT("∞") : FString::FromInt(ReserveAmmo);
    AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %s"), AmmoInMagazine, *ReserveText)));
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# SetWeaponName.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetWeaponName\(const FText& WeaponName\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetWeaponName(const FText& WeaponName)
{
    // Function: Show weapon name only for the infantry weapon panel.
    if (!WeaponText)
    {
        return;
    }

    WeaponText->SetText(PlayerMode == EHeroPlayerMode::Vehicle ? FText::GetEmpty() : WeaponName);
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# SetWeaponHeat.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetWeaponHeat\(const float Heat01\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetWeaponHeat(const float Heat01)
{
    // Function: Hide unused heat/comment-style data unless it is meaningful.
    if (!HeatText)
    {
        return;
    }

    if (PlayerMode == EHeroPlayerMode::Vehicle || Heat01 <= KINDA_SMALL_NUMBER)
    {
        HeatText->SetText(FText::GetEmpty());
        return;
    }

    HeatText->SetText(FText::FromString(FString::Printf(TEXT("HEAT %.0f%%"), FMath::Clamp(Heat01, 0.0f, 1.0f) * 100.0f)));
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# SetVehicleHealth.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetVehicleHealth\(const float Health, const float MaxHealth\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetVehicleHealth(const float Health, const float MaxHealth)
{
    // Function: Vehicle armor is vehicle info, not weapon-panel info.
    if (!VehicleText)
    {
        return;
    }

    if (PlayerMode == EHeroPlayerMode::Vehicle)
    {
        VehicleText->SetText(FText::FromString(FString::Printf(TEXT("CAR ARMOR %.0f / %.0f"), Health, MaxHealth)));
    }
    else
    {
        VehicleText->SetText(FText::GetEmpty());
    }
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# SetVehicleSpeed.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetVehicleSpeed\(const float SpeedKmh\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetVehicleSpeed(const float SpeedKmh)
{
    // Function: Show only actual vehicle speed while driving.
    if (VehicleText && PlayerMode == EHeroPlayerMode::Vehicle)
    {
        VehicleText->SetText(FText::FromString(FString::Printf(TEXT("CAR %.0f km/h"), SpeedKmh)));
    }
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# SetObjectiveStatus.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetObjectiveStatus\(const EHeroGameModeType GameMode, const EHeroTeam Team, const EHeroObjectiveState ObjectiveState, const float ObjectiveProgress01, const bool bContested\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetObjectiveStatus(const EHeroGameModeType GameMode, const EHeroTeam Team, const EHeroObjectiveState ObjectiveState, const float ObjectiveProgress01, const bool bContested)
{
    // Function: Show objective text only when the current mode has a real objective.
    if (ObjectiveText)
    {
        ObjectiveText->SetText(FText::FromString(BuildObjectiveString(GameMode, Team, ObjectiveState, ObjectiveProgress01, bContested)));
    }
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# SetBotStatus.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::SetBotStatus\(const int32 AliveBots, const int32 TotalBots\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::SetBotStatus(const int32 AliveBots, const int32 TotalBots)
{
    // Function: Hide bot counter when there are no bots.
    if (BotText)
    {
        BotText->SetText(TotalBots > 0 ? FText::FromString(FString::Printf(TEXT("BOTS %d/%d"), AliveBots, TotalBots)) : FText::GetEmpty());
    }
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# RefreshModeText.
$HUD = [regex]::Replace(
    $HUD,
    'void UHeroHUDWidget::RefreshModeText\(\)\s*\{.*?\n\}',
@'
void UHeroHUDWidget::RefreshModeText()
{
    // Function: Keep HUD clean. Do not show debug/comment-like mode labels unless driving.
    if (ModeText)
    {
        ModeText->SetText(PlayerMode == EHeroPlayerMode::Vehicle ? FText::FromString(TEXT("DRIVING")) : FText::GetEmpty());
    }

    if (CrosshairText)
    {
        CrosshairText->SetText(FText::FromString(TEXT("+")));
    }

    if (PlayerMode == EHeroPlayerMode::Vehicle)
    {
        if (WeaponText) { WeaponText->SetText(FText::GetEmpty()); }
        if (AmmoText) { AmmoText->SetText(FText::GetEmpty()); }
        if (HeatText) { HeatText->SetText(FText::GetEmpty()); }
    }
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

# BuildObjectiveString.
$HUD = [regex]::Replace(
    $HUD,
    'FString UHeroHUDWidget::BuildObjectiveString\(.*?\)\s*const\s*\{.*?\n\}',
@'
FString UHeroHUDWidget::BuildObjectiveString(const EHeroGameModeType GameMode, const EHeroTeam Team, const EHeroObjectiveState ObjectiveState, const float ObjectiveProgress01, const bool bContested) const
{
    // Function: Main/Sandbox screen should not display objective/debug text.
    if (GameMode == EHeroGameModeType::Sandbox || Team == EHeroTeam::None || ObjectiveState == EHeroObjectiveState::Inactive)
    {
        return FString();
    }

    const TCHAR* ModeName = GameMode == EHeroGameModeType::Escort ? TEXT("ESCORT") : GameMode == EHeroGameModeType::Control ? TEXT("CONTROL") : GameMode == EHeroGameModeType::TeamDeathmatch ? TEXT("TDM") : TEXT("");
    const TCHAR* TeamName = Team == EHeroTeam::TeamA ? TEXT("TEAM A") : Team == EHeroTeam::TeamB ? TEXT("TEAM B") : TEXT("");
    const TCHAR* StateName = bContested || ObjectiveState == EHeroObjectiveState::Contested ? TEXT("CONTESTED") : ObjectiveState == EHeroObjectiveState::Captured ? TEXT("CAPTURED") : ObjectiveState == EHeroObjectiveState::Completed ? TEXT("COMPLETE") : ObjectiveState == EHeroObjectiveState::Active ? TEXT("ACTIVE") : TEXT("");
    return FString::Printf(TEXT("%s  |  %s  |  %s  |  %.0f%%"), ModeName, TeamName, StateName, FMath::Clamp(ObjectiveProgress01, 0.0f, 1.0f) * 100.0f);
}
'@,
    [System.Text.RegularExpressions.RegexOptions]::Singleline)

Write-Utf8NoBom $HUDCpp $HUD

# ---------------------------------------------------------------------
# 3) Main menu cleanup + real patch notes.
#    Remove the "0.1.2 목표" static goal panel; patch notes button fills
#    an actual release note text area.
# ---------------------------------------------------------------------
$MenuH = Get-Content $MenuHeader -Raw -Encoding UTF8
if ($MenuH -notmatch 'PatchNotesText') {
    $MenuH = $MenuH -replace 'TSharedPtr<class STextBlock> StatusText;', "TSharedPtr<class STextBlock> StatusText;`r`n    TSharedPtr<class STextBlock> PatchNotesText;"
}
Write-Utf8NoBom $MenuHeader $MenuH

$MenuCppNew = @'
#include "HeroMainMenuWidget.h"

#include "HeroPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    FSlateFontInfo MenuFont(const int32 Size, const FName Typeface = TEXT("Regular"))
    {
        return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
    }

    FText ReleaseNotes012()
    {
        return FText::FromString(TEXT(
            "v0.1.2 Patch Notes\n"
            "\n"
            "- Sandbox input and mouse control stabilization\n"
            "- Overwatch-style sensitivity yaw 0.0066\n"
            "- Mouse Y / recoil direction separation\n"
            "- Aim zoom FOV transition\n"
            "- Cleaner combat HUD\n"
            "- Direct numeric settings input\n"
            "- Real vehicle-core pawn groundwork\n"
            "- G interact, F melee, Ctrl+S dev vehicle spawn\n"
            "- Infinite reserve ammo for sandbox testing\n"
            "- Cleaner release/function-comment policy"
        ));
    }
}

TSharedRef<SWidget> UHeroMainMenuWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.015f, 0.022f, 0.055f, 1.0f))
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        [
            SNew(SBorder)
            .Padding(FMargin(28.0f, 16.0f))
            .BorderBackgroundColor(FLinearColor(0.005f, 0.010f, 0.030f, 0.90f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("HERO SANDBOX")))
                    .Font(MenuFont(26, TEXT("Bold")))
                    .ColorAndOpacity(FLinearColor(0.0f, 0.75f, 1.0f, 1.0f))
                ]
                + SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right).VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("v0.1.2")))
                    .Font(MenuFont(15))
                    .ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.98f, 1.0f))
                ]
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Center)
        .Padding(72.0f, 90.0f, 0.0f, 48.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("플레이")))
                .Font(MenuFont(52, TEXT("Bold")))
                .ColorAndOpacity(FLinearColor::White)
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 28.0f)
            [
                SAssignNew(StatusText, STextBlock)
                .Text(FText::FromString(TEXT("모드를 선택하십시오.")))
                .Font(MenuFont(17))
                .ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.98f, 1.0f))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandlePlaySandboxClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("플레이 샌드박스"))).Font(MenuFont(24, TEXT("Bold")))]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCombatTestClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("전투 테스트"))).Font(MenuFont(24, TEXT("Bold")))]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCustomGameClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("사용자 지정 게임"))).Font(MenuFont(24, TEXT("Bold")))]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 22.0f, 0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleSettingsClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("설정"))).Font(MenuFont(22))]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCreditsClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("패치 노트"))).Font(MenuFont(22))]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleQuitClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("게임 종료"))).Font(MenuFont(22))]
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Center)
        .Padding(0.0f, 90.0f, 72.0f, 48.0f)
        [
            SNew(SBorder)
            .Padding(26.0f)
            .BorderBackgroundColor(FLinearColor(0.0f, 0.02f, 0.06f, 0.62f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("PATCH NOTES")))
                    .Font(MenuFont(27, TEXT("Bold")))
                    .ColorAndOpacity(FLinearColor(0.0f, 0.72f, 1.0f, 1.0f))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 0.0f)
                [
                    SAssignNew(PatchNotesText, STextBlock)
                    .Text(FText::FromString(TEXT("")))
                    .Font(MenuFont(17))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ];
}

FReply UHeroMainMenuWidget::HandlePlaySandboxClicked()
{
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->StartSandboxFromMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleCombatTestClicked()
{
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->StartCombatTestFromMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleCustomGameClicked()
{
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->StartCustomGameFromMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleSettingsClicked()
{
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->ShowSettingsMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleCreditsClicked()
{
    // Function: Patch notes button displays the real release notes instead of a placeholder sentence.
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("v0.1.2 패치 노트")));
    }
    if (PatchNotesText)
    {
        PatchNotesText->SetText(ReleaseNotes012());
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleQuitClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
    return FReply::Handled();
}
'@

Write-Utf8NoBom $MenuCpp $MenuCppNew

# ---------------------------------------------------------------------
# 4) Crouch visual smoothing.
# ---------------------------------------------------------------------
$HeroH = Get-Content $HeroHeader -Raw -Encoding UTF8
$HeroH = $HeroH -replace 'float CrouchWalkSpeed\s*=\s*[0-9.]+f\s*;', 'float CrouchWalkSpeed = 300.0f;'
$HeroH = $HeroH -replace 'float HeroCrouchedEyeHeight\s*=\s*[0-9.]+f\s*;', 'float HeroCrouchedEyeHeight = 46.0f;'
$HeroH = $HeroH -replace 'float CameraHeightInterpSpeed\s*=\s*[0-9.]+f\s*;', 'float CameraHeightInterpSpeed = 7.5f;'
Write-Utf8NoBom $HeroHeader $HeroH

$HeroC = Get-Content $HeroCpp -Raw -Encoding UTF8
$HeroC = $HeroC -replace 'Move->MaxWalkSpeedCrouched\s*=\s*CrouchWalkSpeed\s*;', 'Move->MaxWalkSpeedCrouched = CrouchWalkSpeed;'
$HeroC = $HeroC -replace 'Movement->MaxWalkSpeedCrouched\s*=\s*CrouchWalkSpeed\s*;', 'Movement->MaxWalkSpeedCrouched = CrouchWalkSpeed;'
Write-Utf8NoBom $HeroCpp $HeroC

# ---------------------------------------------------------------------
# 5) Vehicle coast after exit, if real vehicle pawn exists.
# ---------------------------------------------------------------------
if (Test-Path $VehicleCpp) {
    $Vehicle = Get-Content $VehicleCpp -Raw -Encoding UTF8

    $Vehicle = [regex]::Replace(
        $Vehicle,
        'void AHeroDriveableVehiclePawn::Tick\(const float DeltaSeconds\)\s*\{.*?\n\}',
@'
void AHeroDriveableVehiclePawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Function: Vehicle physics keeps simulating after exit, so the car coasts naturally instead of freezing.
    if (!HasDriver())
    {
        ForwardInput = 0.0f;
        SteeringInput = 0.0f;
        bHandbrakeInput = 0;
    }

    PushInputToVehicleCore(DeltaSeconds);
    StepVehicleCore(DeltaSeconds);
    ApplyVehicleCoreTransform();
}
'@,
        [System.Text.RegularExpressions.RegexOptions]::Singleline)

    $Vehicle = $Vehicle -replace 'PlayerInputComponent->BindAction\(TEXT\("ToggleVehicleMode"\), IE_Pressed, this, &AHeroDriveableVehiclePawn::RequestExitVehicle\);', ''
    $Vehicle = $Vehicle -replace 'PlayerInputComponent->BindAction\(TEXT\("Interact"\), IE_Pressed, this, &AHeroDriveableVehiclePawn::RequestExitVehicle\);', 'PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AHeroDriveableVehiclePawn::RequestExitVehicle);'

    Write-Utf8NoBom $VehicleCpp $Vehicle
}

# ---------------------------------------------------------------------
# 6) If real vehicle pawn exists, prevent PlayerController from putting vehicle
#    info into weapon name/ammo panel.
# ---------------------------------------------------------------------
if (Test-Path $VehicleCpp) {
    $PC = Get-Content $PlayerControllerCpp -Raw -Encoding UTF8

    if ($PC -notmatch '#include "HeroDriveableVehiclePawn.h"') {
        $PC = $PC -replace '#include "HeroCharacter.h"', "#include `"HeroCharacter.h`"`r`n#include `"HeroDriveableVehiclePawn.h`""
    }

    $PC = [regex]::Replace(
        $PC,
        'void AHeroPlayerController::UpdateHUDFromPawn\(\)\s*\{.*?\n\}',
@'
void AHeroPlayerController::UpdateHUDFromPawn()
{
    if (!HeroHUD)
    {
        return;
    }

    if (AHeroDriveableVehiclePawn* VehiclePawn = Cast<AHeroDriveableVehiclePawn>(GetPawn()))
    {
        // Function: Driving HUD shows vehicle data only in vehicle area, not in the weapon panel.
        HeroHUD->SetPlayerMode(EHeroPlayerMode::Vehicle);
        HeroHUD->SetHealth(VehiclePawn->GetVehicleHealth(), VehiclePawn->GetMaxVehicleHealth());
        HeroHUD->SetVehicleHealth(VehiclePawn->GetVehicleHealth(), VehiclePawn->GetMaxVehicleHealth());
        HeroHUD->SetVehicleSpeed(VehiclePawn->GetSpeedKmh());
        HeroHUD->SetWeaponName(FText::GetEmpty());
        HeroHUD->SetAmmo(-1, -1);
        HeroHUD->SetWeaponHeat(0.0f);
        return;
    }

    AHeroCharacter* Hero = Cast<AHeroCharacter>(GetPawn());
    if (!Hero)
    {
        return;
    }

    UHeroHealthComponent* Health = Hero->GetHealthComponent();
    UHeroWeaponComponent* Weapon = Hero->GetWeaponComponent();
    UHeroVehicleModeComponent* VehicleMode = Hero->GetVehicleModeComponent();
    UHeroTeamComponent* Team = Hero->GetTeamComponent();

    if (VehicleMode)
    {
        HeroHUD->SetPlayerMode(VehicleMode->GetPlayerMode());
    }
    if (Health)
    {
        HeroHUD->SetHealth(Health->GetHealth(), Health->GetMaxHealth());
        HeroHUD->SetVehicleHealth(0.0f, 0.0f);
    }
    if (Weapon)
    {
        HeroHUD->SetWeaponName(Weapon->GetWeaponName());
        HeroHUD->SetAmmo(Weapon->GetAmmoInMagazine(), Weapon->GetReserveAmmo());
        HeroHUD->SetReloading(Weapon->IsReloading());
        HeroHUD->SetWeaponHeat(Weapon->GetWeaponHeat01());
    }

    if (const AHeroGameModeBase* HeroGM = GetWorld() ? GetWorld()->GetAuthGameMode<AHeroGameModeBase>() : nullptr)
    {
        HeroHUD->SetObjectiveStatus(
            HeroGM->GetCustomGameSettings().GameModeType,
            Team ? Team->GetTeam() : EHeroTeam::None,
            HeroGM->GetPrimaryObjectiveState(),
            HeroGM->GetPrimaryObjectiveProgress01(),
            HeroGM->GetPrimaryObjectiveState() == EHeroObjectiveState::Contested);
        HeroHUD->SetBotStatus(HeroGM->GetAliveBotCount(), HeroGM->GetTotalSpawnedBotCount());
    }
}
'@,
        [System.Text.RegularExpressions.RegexOptions]::Singleline)

    Write-Utf8NoBom $PlayerControllerCpp $PC
}

Write-Host "Applied ammo / vehicle coast / clean HUD / patch notes / crouch smoothing fix."
Write-Host "  - Reserve ammo is infinite"
Write-Host "  - Vehicle coasts after exit if real vehicle pawn exists"
Write-Host "  - Vehicle info no longer overwrites weapon panel"
Write-Host "  - Main menu target panel removed"
Write-Host "  - Patch notes button shows real v0.1.2 notes"
Write-Host "  - Crouch camera transition is slower and smoother"
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

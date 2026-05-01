
param(
    [string]$ProjectRoot = "D:\UnrealProject\HeroVehicleSandbox",
    [string]$UERoot = "D:\UE_5.7"
)

$ErrorActionPreference = "Stop"

$SourceRoot = Join-Path $ProjectRoot "Source\HeroVehicleSandbox"
$HeroTypesPath = Join-Path $SourceRoot "Public\HeroTypes.h"
$HeroCharacterPath = Join-Path $SourceRoot "Private\HeroCharacter.cpp"
$PlayerControllerHeaderPath = Join-Path $SourceRoot "Public\HeroPlayerController.h"
$PlayerControllerCppPath = Join-Path $SourceRoot "Private\HeroPlayerController.cpp"
$SettingsWidgetPath = Join-Path $SourceRoot "Private\HeroSettingsWidget.cpp"
$GameModeHeaderPath = Join-Path $SourceRoot "Public\HeroGameModeBase.h"
$GameModeCppPath = Join-Path $SourceRoot "Private\HeroGameModeBase.cpp"
$HUDCppPath = Join-Path $SourceRoot "Private\HeroHUDWidget.cpp"
$MenuCppPath = Join-Path $SourceRoot "Private\HeroMainMenuWidget.cpp"
$InputPath = Join-Path $ProjectRoot "Config\DefaultInput.ini"
$ProjectPath = Join-Path $ProjectRoot "HeroVehicleSandbox.uproject"
$LogPath = Join-Path $ProjectRoot "Build_HeroVehicleSandboxEditor.log"

foreach ($Path in @($HeroTypesPath, $HeroCharacterPath, $PlayerControllerHeaderPath, $PlayerControllerCppPath, $SettingsWidgetPath, $InputPath)) {
    if (!(Test-Path $Path)) {
        throw "Required file not found: $Path"
    }

    $Backup = "$Path.input_sens_fix_v3.bak"
    if (!(Test-Path $Backup)) {
        Copy-Item $Path $Backup
    }
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

function Replace-FHeroAimSettings([string]$Text, [string]$Replacement) {
    $StructIndex = $Text.IndexOf("struct FHeroAimSettings")
    if ($StructIndex -lt 0) {
        throw "FHeroAimSettings struct not found in HeroTypes.h"
    }

    $BeginIndex = $Text.LastIndexOf("USTRUCT(BlueprintType)", $StructIndex)
    if ($BeginIndex -lt 0) {
        $BeginIndex = $StructIndex
    }

    $EndIndex = $Text.IndexOf("};", $StructIndex)
    if ($EndIndex -lt 0) {
        throw "Could not find end of FHeroAimSettings in HeroTypes.h"
    }

    $Before = $Text.Substring(0, $BeginIndex)
    $After = $Text.Substring($EndIndex + 2)
    return $Before + $Replacement + $After
}

# ------------------------------------------------------------
# 1) HeroTypes.h: robust replacement, not regex-dependent.
# ------------------------------------------------------------
$HeroTypes = Get-Content $HeroTypesPath -Raw -Encoding UTF8

$NewAimSettings = @'
USTRUCT(BlueprintType)
struct FHeroAimSettings
{
    GENERATED_BODY()

    // Overwatch-style yaw.
    //
    // Runtime:
    //   degrees/count = MouseSensitivity * 0.0066
    //
    // cm/360:
    //   cm/360 = 360 * 2.54 / (DPI * MouseSensitivity * 0.0066)
    //          ~= 138600 / (DPI * MouseSensitivity)
    //
    // DPI is only for physical-distance display. Do NOT multiply DPI into
    // runtime camera rotation.
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

$HeroTypesNew = Replace-FHeroAimSettings $HeroTypes $NewAimSettings
Write-Utf8NoBom $HeroTypesPath $HeroTypesNew

# ------------------------------------------------------------
# 2) HeroCharacter.cpp: direct ControlRotation look.
# ------------------------------------------------------------
$HeroCharacter = Get-Content $HeroCharacterPath -Raw -Encoding UTF8

if ($HeroCharacter -notmatch '#include "GameFramework/Controller.h"') {
    $HeroCharacter = $HeroCharacter -replace '#include "GameFramework/CharacterMovementComponent.h"', "#include `"GameFramework/CharacterMovementComponent.h`"`r`n#include `"GameFramework/Controller.h`""
}

$NewTurn = @'
void AHeroCharacter::Turn(const float Value)
{
    if (Controller && Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
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
    if (Controller && Value != 0.0f && (!HealthComponent || !HealthComponent->IsDead()))
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

if ($HeroCharacter -match 'void AHeroCharacter::Turn\(const float Value\)') {
    $HeroCharacter = [regex]::Replace($HeroCharacter, 'void AHeroCharacter::Turn\(const float Value\)\s*\{.*?\n\}', $NewTurn, [System.Text.RegularExpressions.RegexOptions]::Singleline)
} else {
    $HeroCharacter = $HeroCharacter.TrimEnd() + "`r`n`r`n" + $NewTurn
}

if ($HeroCharacter -match 'void AHeroCharacter::LookUp\(const float Value\)') {
    $HeroCharacter = [regex]::Replace($HeroCharacter, 'void AHeroCharacter::LookUp\(const float Value\)\s*\{.*?\n\}', $NewLookUp, [System.Text.RegularExpressions.RegexOptions]::Singleline)
} else {
    $HeroCharacter = $HeroCharacter.TrimEnd() + "`r`n`r`n" + $NewLookUp
}

Write-Utf8NoBom $HeroCharacterPath $HeroCharacter

# ------------------------------------------------------------
# 3) PlayerController: game input mode and ESC settings toggle.
# ------------------------------------------------------------
$PCH = Get-Content $PlayerControllerHeaderPath -Raw -Encoding UTF8
if ($PCH -notmatch 'SetupInputComponent\(\) override') {
    $PCH = $PCH -replace 'virtual void Tick\(float DeltaSeconds\) override;', "virtual void Tick(float DeltaSeconds) override;`r`n    virtual void SetupInputComponent() override;"
}
if ($PCH -notmatch 'ToggleSettingsMenu') {
    $PCH = $PCH -replace 'void StartSandboxFromMenu\(\);', "void StartSandboxFromMenu();`r`n`r`n    UFUNCTION(BlueprintCallable, Category=`"Hero|UI`")`r`n    void ToggleSettingsMenu();"
}
Write-Utf8NoBom $PlayerControllerHeaderPath $PCH

$PCC = Get-Content $PlayerControllerCppPath -Raw -Encoding UTF8

foreach ($Inc in @(
    '#include "Engine/Engine.h"',
    '#include "Engine/GameViewportClient.h"',
    '#include "Framework/Application/SlateApplication.h"'
)) {
    if ($PCC -notmatch [regex]::Escape($Inc)) {
        $PCC = $PCC -replace '#include "GameFramework/Pawn.h"', "#include `"GameFramework/Pawn.h`"`r`n$Inc"
    }
}

$NewShowHUD = @'
void AHeroPlayerController::ShowHUD()
{
    EnsureHeroPawnPossessed();
    ClearMenu();

    if (!HeroHUD)
    {
        UClass* WidgetClass = HUDWidgetClass ? HUDWidgetClass.Get() : UHeroHUDWidget::StaticClass();
        HeroHUD = CreateWidget<UHeroHUDWidget>(this, WidgetClass);
    }

    if (HeroHUD && !HeroHUD->IsInViewport())
    {
        HeroHUD->AddToViewport(5);
    }

    SetGameInputMode();
}
'@

$NewStartSandbox = @'
void AHeroPlayerController::StartSandboxFromMenu()
{
    EnsureHeroPawnPossessed();

    if (APawn* ControlledPawn = GetPawn())
    {
        ControlledPawn->EnableInput(this);
    }

    ShowHUD();
}
'@

$NewSetGameInputMode = @'
void AHeroPlayerController::SetGameInputMode()
{
    EnsureHeroPawnPossessed();

    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;

    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);
    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();

    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    SetInputMode(InputMode);

    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        GEngine->GameViewport->SetMouseLockMode(EMouseLockMode::LockAlways);
    }

    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::SetDirectly);
    }

    FlushPressedKeys();
}
'@

$PCC = [regex]::Replace($PCC, 'void AHeroPlayerController::ShowHUD\(\)\s*\{.*?\n\}', $NewShowHUD, [System.Text.RegularExpressions.RegexOptions]::Singleline)
$PCC = [regex]::Replace($PCC, 'void AHeroPlayerController::StartSandboxFromMenu\(\)\s*\{.*?\n\}', $NewStartSandbox, [System.Text.RegularExpressions.RegexOptions]::Singleline)
$PCC = [regex]::Replace($PCC, 'void AHeroPlayerController::SetGameInputMode\(\)\s*\{.*?\n\}', $NewSetGameInputMode, [System.Text.RegularExpressions.RegexOptions]::Singleline)

if ($PCC -notmatch 'void AHeroPlayerController::SetupInputComponent\(\)') {
$SetupFunc = @'

void AHeroPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (InputComponent)
    {
        InputComponent->BindAction(TEXT("PauseMenu"), IE_Pressed, this, &AHeroPlayerController::ToggleSettingsMenu);
    }
}
'@
    $PCC = $PCC.TrimEnd() + "`r`n" + $SetupFunc
}

if ($PCC -notmatch 'void AHeroPlayerController::ToggleSettingsMenu\(\)') {
$ToggleFunc = @'

void AHeroPlayerController::ToggleSettingsMenu()
{
    if (CurrentMenu)
    {
        ClearMenu();
        ShowHUD();
        return;
    }

    ShowSettingsMenu();
}
'@
    $PCC = $PCC.TrimEnd() + "`r`n" + $ToggleFunc
}

Write-Utf8NoBom $PlayerControllerCppPath $PCC

# ------------------------------------------------------------
# 4) Settings UI: spinbox + direct editable numeric boxes.
# ------------------------------------------------------------
$SettingsWidgetCpp = @'
#include "HeroSettingsWidget.h"

#include "HeroCharacter.h"
#include "HeroPlayerController.h"
#include "HeroSettingsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UHeroSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LoadSettings();
}

TSharedRef<SWidget> UHeroSettingsWidget::RebuildWidget()
{
    LoadSettings();

    auto ParseFloat = [](const FText& Text, const float Fallback)
    {
        float Parsed = Fallback;
        if (LexTryParseString(Parsed, *Text.ToString()))
        {
            return Parsed;
        }
        return Fallback;
    };

    auto MakeNumberRow =
        [this, ParseFloat](
            const FString& Label,
            const FString& Hint,
            TFunction<float()> Getter,
            TFunction<void(float)> Setter,
            const float MinValue,
            const float MaxValue,
            const float Step)
        -> TSharedRef<SWidget>
    {
        return SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .FillWidth(0.34f)
            .VAlign(VAlign_Center)
            .Padding(0.0f, 6.0f, 12.0f, 6.0f)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Label))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Hint))
                ]
            ]

            + SHorizontalBox::Slot()
            .FillWidth(0.34f)
            .Padding(4.0f, 6.0f)
            [
                SNew(SSpinBox<float>)
                .MinValue(MinValue)
                .MaxValue(MaxValue)
                .Delta(Step)
                .Value_Lambda([Getter]()
                {
                    return Getter();
                })
                .OnValueChanged_Lambda([Setter](const float Value)
                {
                    Setter(Value);
                })
                .OnValueCommitted_Lambda([Setter](const float Value, ETextCommit::Type)
                {
                    Setter(Value);
                })
            ]

            + SHorizontalBox::Slot()
            .FillWidth(0.32f)
            .Padding(4.0f, 6.0f)
            [
                SNew(SEditableTextBox)
                .SelectAllTextWhenFocused(true)
                .RevertTextOnEscape(true)
                .HintText(FText::FromString(TEXT("Type value")))
                .Text_Lambda([Getter]()
                {
                    return FText::AsNumber(Getter());
                })
                .OnTextCommitted_Lambda([Getter, Setter, ParseFloat, MinValue, MaxValue](const FText& Text, ETextCommit::Type)
                {
                    const float Parsed = ParseFloat(Text, Getter());
                    Setter(FMath::Clamp(Parsed, MinValue, MaxValue));
                })
            ];
    };

    return SNew(SBorder)
        .Padding(36.0f)
        [
            SNew(SScrollBox)

            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 16.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("SETTINGS")))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 18.0f)
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        return FText::FromString(FString::Printf(
                            TEXT("Overwatch-style yaw 0.0066  |  cm/360 = 138600 / (DPI * Sens)  |  Current %.2f cm/360"),
                            WorkingAimSettings.GetCmPer360()));
                    })
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeNumberRow(
                        TEXT("Mouse Sensitivity"),
                        TEXT("Runtime: degrees/count = Sens * 0.0066"),
                        [this]() { return WorkingAimSettings.MouseSensitivity; },
                        [this](const float V) { SetMouseSensitivity(V); },
                        0.01f,
                        100.0f,
                        0.01f)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeNumberRow(
                        TEXT("DPI for cm/360 display"),
                        TEXT("Does not change runtime rotation"),
                        [this]() { return WorkingAimSettings.ReferenceMouseDpi; },
                        [this](const float V) { WorkingAimSettings.ReferenceMouseDpi = FMath::Clamp(V, 100.0f, 64000.0f); },
                        100.0f,
                        64000.0f,
                        50.0f)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeNumberRow(
                        TEXT("Scoped Multiplier"),
                        TEXT("ADS multiplier"),
                        [this]() { return WorkingAimSettings.ScopedSensitivityMultiplier; },
                        [this](const float V) { WorkingAimSettings.ScopedSensitivityMultiplier = FMath::Clamp(V, 0.1f, 2.0f); },
                        0.1f,
                        2.0f,
                        0.01f)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeNumberRow(
                        TEXT("Vehicle Look Multiplier"),
                        TEXT("Vehicle mode mouse multiplier"),
                        [this]() { return WorkingAimSettings.VehicleLookSensitivityMultiplier; },
                        [this](const float V) { WorkingAimSettings.VehicleLookSensitivityMultiplier = FMath::Clamp(V, 0.1f, 2.0f); },
                        0.1f,
                        2.0f,
                        0.01f)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    MakeNumberRow(
                        TEXT("First Person FOV"),
                        TEXT("80-120 degrees"),
                        [this]() { return WorkingAimSettings.FirstPersonFovDegrees; },
                        [this](const float V) { SetFov(V); },
                        80.0f,
                        120.0f,
                        1.0f)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 12.0f)
                [
                    SNew(SCheckBox)
                    .IsChecked(WorkingAimSettings.bInvertMouseY ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                    {
                        WorkingAimSettings.bInvertMouseY = State == ECheckBoxState::Checked;
                    })
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("Invert Mouse Y")))
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 18.0f, 0.0f, 0.0f)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(4.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Apply")))
                        .OnClicked_UObject(this, &UHeroSettingsWidget::HandleApplyClicked)
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(4.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Reset to Default")))
                        .OnClicked_UObject(this, &UHeroSettingsWidget::HandleResetClicked)
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(4.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Back")))
                        .OnClicked_UObject(this, &UHeroSettingsWidget::HandleBackClicked)
                    ]
                ]
            ]
        ];
}

void UHeroSettingsWidget::SetMouseSensitivity(const float NewSensitivity)
{
    WorkingAimSettings.MouseSensitivity = FMath::Clamp(NewSensitivity, 0.01f, 100.0f);
}

float UHeroSettingsWidget::GetMouseSensitivity() const
{
    return WorkingAimSettings.MouseSensitivity;
}

void UHeroSettingsWidget::SetFov(const float NewFov)
{
    WorkingAimSettings.FirstPersonFovDegrees = FMath::Clamp(NewFov, 80.0f, 120.0f);
}

float UHeroSettingsWidget::GetFov() const
{
    return WorkingAimSettings.FirstPersonFovDegrees;
}

void UHeroSettingsWidget::ApplySettings()
{
    WorkingAimSettings.Clamp();
    UHeroSettingsSaveGame* Save = Cast<UHeroSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("HeroVehicleSettings"), 0));
    if (!Save)
    {
        Save = Cast<UHeroSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UHeroSettingsSaveGame::StaticClass()));
    }

    if (Save)
    {
        Save->AimSettings = WorkingAimSettings;
        UGameplayStatics::SaveGameToSlot(Save, TEXT("HeroVehicleSettings"), 0);
    }

    if (AHeroCharacter* Hero = GetOwningPlayerPawn<AHeroCharacter>())
    {
        Hero->ApplyAimSettings(WorkingAimSettings);
    }
}

void UHeroSettingsWidget::ResetToDefaults()
{
    WorkingAimSettings = FHeroAimSettings{};
    ApplySettings();
}

void UHeroSettingsWidget::LoadSettings()
{
    if (UHeroSettingsSaveGame* Save = Cast<UHeroSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("HeroVehicleSettings"), 0)))
    {
        WorkingAimSettings = Save->AimSettings;
    }
    WorkingAimSettings.Clamp();
}

FReply UHeroSettingsWidget::HandleApplyClicked()
{
    ApplySettings();
    return FReply::Handled();
}

FReply UHeroSettingsWidget::HandleResetClicked()
{
    ResetToDefaults();
    return FReply::Handled();
}

FReply UHeroSettingsWidget::HandleBackClicked()
{
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        if (PC->GetHeroHUD() && PC->GetHeroHUD()->IsInViewport())
        {
            PC->ShowHUD();
        }
        else
        {
            PC->ShowMainMenu();
        }
    }
    return FReply::Handled();
}
'@
Write-Utf8NoBom $SettingsWidgetPath $SettingsWidgetCpp

# ------------------------------------------------------------
# 5) Config: legacy input classes + mouse axis scale 1.
# ------------------------------------------------------------
$Input = Get-Content $InputPath -Raw -Encoding UTF8

$Input = $Input -replace 'AxisKeyName="MouseX",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="MouseX",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="MouseY",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="MouseY",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="Mouse2D",AxisProperties=\(DeadZone=0\.000000,Sensitivity=[0-9.]+,Exponent=1\.000000,bInvert=False\)', 'AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False)'
$Input = $Input -replace 'AxisKeyName="MouseX",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="MouseX",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'
$Input = $Input -replace 'AxisKeyName="MouseY",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="MouseY",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'
$Input = $Input -replace 'AxisKeyName="Mouse2D",AxisProperties=\(DeadZone=0\.f,Exponent=1\.f,Sensitivity=[0-9.]+f\)', 'AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=1.f)'

$Input = $Input -replace 'bEnableFOVScaling=True', 'bEnableFOVScaling=False'
$Input = $Input -replace 'bEnableLegacyInputScales=True', 'bEnableLegacyInputScales=False'

if ($Input -match 'DefaultPlayerInputClass=') {
    $Input = $Input -replace 'DefaultPlayerInputClass=.*', 'DefaultPlayerInputClass=/Script/Engine.PlayerInput'
}
else {
    $Input += "`r`nDefaultPlayerInputClass=/Script/Engine.PlayerInput`r`n"
}
if ($Input -match 'DefaultInputComponentClass=') {
    $Input = $Input -replace 'DefaultInputComponentClass=.*', 'DefaultInputComponentClass=/Script/Engine.InputComponent'
}
else {
    $Input += "`r`nDefaultInputComponentClass=/Script/Engine.InputComponent`r`n"
}

if ($Input -notmatch 'ActionName="PauseMenu"') {
    $Input += "`r`n+ActionMappings=(ActionName=`"PauseMenu`",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Escape)`r`n"
}

Write-Utf8NoBom $InputPath $Input

# ------------------------------------------------------------
# 6) Carry-forward known compile fixes.
# ------------------------------------------------------------
if (Test-Path $GameModeHeaderPath) {
    $Header = Get-Content $GameModeHeaderPath -Raw -Encoding UTF8
    $Header = $Header -replace 'void SpawnBot\(EHeroTeam Team, EHeroBotRole Role, const FVector& Location, const FRotator& Rotation\);',
                               'void SpawnBot(EHeroTeam Team, EHeroBotRole BotRole, const FVector& Location, const FRotator& Rotation);'
    Write-Utf8NoBom $GameModeHeaderPath $Header
}
if (Test-Path $GameModeCppPath) {
    $GM = Get-Content $GameModeCppPath -Raw -Encoding UTF8
    $GM = $GM -replace 'const EHeroBotRole Role =', 'const EHeroBotRole BotRole ='
    $GM = $GM -replace 'SpawnBot\(EHeroTeam::TeamA, Role,', 'SpawnBot(EHeroTeam::TeamA, BotRole,'
    $GM = $GM -replace 'SpawnBot\(EHeroTeam::TeamB, Role,', 'SpawnBot(EHeroTeam::TeamB, BotRole,'
    $GM = $GM -replace 'SpawnBot\(Team, Role,', 'SpawnBot(Team, BotRole,'
    $GM = $GM -replace 'void AHeroGameModeBase::SpawnBot\(const EHeroTeam Team, const EHeroBotRole Role, const FVector& Location, const FRotator& Rotation\)',
                        'void AHeroGameModeBase::SpawnBot(const EHeroTeam Team, const EHeroBotRole BotRole, const FVector& Location, const FRotator& Rotation)'
    $GM = $GM -replace '\bSetBotRole\(Role\)', 'SetBotRole(BotRole)'
    $GM = $GM -replace '\bInitializeBot\(Team, Role\)', 'InitializeBot(Team, BotRole)'
    $GM = $GM -replace '\bSetTeamAndRole\(Team, Role\)', 'SetTeamAndRole(Team, BotRole)'
    Write-Utf8NoBom $GameModeCppPath $GM
}

foreach ($Path in @($HUDCppPath, $MenuCppPath)) {
    if (Test-Path $Path) {
        $Text = Get-Content $Path -Raw -Encoding UTF8
        $Text = $Text -replace '#include "Widgets/Layout/SOverlay.h"', '#include "Widgets/SOverlay.h"'
        Write-Utf8NoBom $Path $Text
    }
}

Write-Host "Applied input/sensitivity/settings fix v3."
Write-Host "Runtime sensitivity = in-game sensitivity * 0.0066 degrees/count"
Write-Host "cm/360 = 138600 / (DPI * sensitivity)"
Write-Host "Legacy input classes restored."
Write-Host "Settings has direct numeric text boxes."
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

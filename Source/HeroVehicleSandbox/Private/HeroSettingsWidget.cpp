#include "HeroSettingsWidget.h"

#include "HeroCharacter.h"
#include "HeroPlayerController.h"
#include "HeroSettingsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
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

    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f)
        [SNew(STextBlock).Text(FText::FromString(TEXT("Settings")))]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(STextBlock).Text(FText::FromString(TEXT("Default: 1600 DPI / Sens 1.0 / 86.6 cm per 360")))]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 8.0f)
        [SNew(STextBlock).Text(FText::FromString(TEXT("Controls")))]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(SSpinBox<float>).MinValue(0.1f).MaxValue(20.0f).Value(WorkingAimSettings.MouseSensitivity).OnValueChanged_Lambda([this](float V){ SetMouseSensitivity(V); })]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(SSpinBox<float>).MinValue(0.1f).MaxValue(2.0f).Value(WorkingAimSettings.ScopedSensitivityMultiplier).OnValueChanged_Lambda([this](float V){ WorkingAimSettings.ScopedSensitivityMultiplier = V; })]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(SSpinBox<float>).MinValue(0.1f).MaxValue(2.0f).Value(WorkingAimSettings.VehicleLookSensitivityMultiplier).OnValueChanged_Lambda([this](float V){ WorkingAimSettings.VehicleLookSensitivityMultiplier = V; })]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 8.0f)
        [SNew(STextBlock).Text(FText::FromString(TEXT("Video")))]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(SSpinBox<float>).MinValue(80.0f).MaxValue(120.0f).Value(WorkingAimSettings.FirstPersonFovDegrees).OnValueChanged_Lambda([this](float V){ SetFov(V); })]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(SCheckBox).IsChecked(WorkingAimSettings.bInvertMouseY ? ECheckBoxState::Checked : ECheckBoxState::Unchecked).OnCheckStateChanged_Lambda([this](ECheckBoxState S){ WorkingAimSettings.bInvertMouseY = S == ECheckBoxState::Checked; })
            [SNew(STextBlock).Text(FText::FromString(TEXT("Invert Mouse Y")))]]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 8.0f)
        [SNew(STextBlock).Text(FText::FromString(TEXT("Audio: Master / Weapon / Vehicle / UI Volume placeholders")))]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 4.0f)
        [SNew(STextBlock).Text(FText::FromString(TEXT("Gameplay: Damage Numbers / Crosshair / Vehicle Hints / Auto Reload placeholders")))]
        + SVerticalBox::Slot().AutoHeight().Padding(24.0f, 12.0f)
        [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("Apply"))).OnClicked_UObject(this, &UHeroSettingsWidget::HandleApplyClicked)]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("Reset to Default"))).OnClicked_UObject(this, &UHeroSettingsWidget::HandleResetClicked)]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f)[SNew(SButton).Text(FText::FromString(TEXT("Back"))).OnClicked_UObject(this, &UHeroSettingsWidget::HandleBackClicked)]
        ];
}

void UHeroSettingsWidget::SetMouseSensitivity(const float NewSensitivity)
{
    WorkingAimSettings.MouseSensitivity = FMath::Clamp(NewSensitivity, 0.1f, 20.0f);
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
        PC->ShowMainMenu();
    }
    return FReply::Handled();
}

#include "HeroSettingsWidget.h"

#include "HeroCharacter.h"
#include "HeroHUDWidget.h"
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

// 기능: 설정 위젯 생성 시 저장된 설정을 불러온다.
void UHeroSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    LoadSettings();
}

// 기능: 감도/FOV 설정 UI를 SpinBox와 직접 숫자 입력 박스로 구성한다.
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
// 기능: Apply 버튼 클릭 시 설정을 저장하고 적용한다.
                        .OnClicked_UObject(this, &UHeroSettingsWidget::HandleApplyClicked)
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(4.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Reset to Default")))
// 기능: Reset 버튼 클릭 시 설정을 기본값으로 되돌린다.
                        .OnClicked_UObject(this, &UHeroSettingsWidget::HandleResetClicked)
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(4.0f)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Back")))
// 기능: Back 버튼 클릭 시 HUD 또는 메인 메뉴로 복귀한다.
                        .OnClicked_UObject(this, &UHeroSettingsWidget::HandleBackClicked)
                    ]
                ]
            ]
        ];
}

// 기능: 마우스 감도 값을 허용 범위로 제한해 임시 설정에 반영한다.
void UHeroSettingsWidget::SetMouseSensitivity(const float NewSensitivity)
{
    WorkingAimSettings.MouseSensitivity = FMath::Clamp(NewSensitivity, 0.01f, 100.0f);
}

// 기능: 현재 임시 마우스 감도 값을 반환한다.
float UHeroSettingsWidget::GetMouseSensitivity() const
{
    return WorkingAimSettings.MouseSensitivity;
}

// 기능: 1인칭 FOV 값을 허용 범위로 제한해 임시 설정에 반영한다.
void UHeroSettingsWidget::SetFov(const float NewFov)
{
    WorkingAimSettings.FirstPersonFovDegrees = FMath::Clamp(NewFov, 80.0f, 120.0f);
}

// 기능: 현재 임시 FOV 값을 반환한다.
float UHeroSettingsWidget::GetFov() const
{
    return WorkingAimSettings.FirstPersonFovDegrees;
}

// 기능: 임시 설정을 저장 슬롯에 기록하고 현재 영웅 캐릭터에 즉시 적용한다.
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

// 기능: 조준 설정을 기본값으로 되돌리고 저장/적용한다.
void UHeroSettingsWidget::ResetToDefaults()
{
    WorkingAimSettings = FHeroAimSettings{};
    ApplySettings();
}

// 기능: 저장 슬롯에서 설정을 불러와 작업용 설정에 복사한다.
void UHeroSettingsWidget::LoadSettings()
{
    if (UHeroSettingsSaveGame* Save = Cast<UHeroSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("HeroVehicleSettings"), 0)))
    {
        WorkingAimSettings = Save->AimSettings;
    }
    WorkingAimSettings.Clamp();
}

// 기능: Apply 버튼 클릭 시 설정을 저장하고 적용한다.
FReply UHeroSettingsWidget::HandleApplyClicked()
{
    ApplySettings();
    return FReply::Handled();
}

// 기능: Reset 버튼 클릭 시 설정을 기본값으로 되돌린다.
FReply UHeroSettingsWidget::HandleResetClicked()
{
    ResetToDefaults();
    return FReply::Handled();
}

// 기능: Back 버튼 클릭 시 HUD 또는 메인 메뉴로 복귀한다.
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

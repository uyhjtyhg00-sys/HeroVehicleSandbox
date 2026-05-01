#include "HeroHUDWidget.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UHeroHUDWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SAssignNew(CrosshairText, STextBlock)
            .Text(FText::FromString(TEXT("+")))
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Bottom)
        .Padding(24.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(HealthText, STextBlock).Text(FText::FromString(TEXT("Health 200 / 200")))]
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(ModeText, STextBlock).Text(FText::FromString(TEXT("HUMAN MODE")))]
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(ObjectiveText, STextBlock).Text(FText::FromString(TEXT("Sandbox | Objective inactive")))]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Bottom)
        .Padding(24.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(WeaponText, STextBlock).Text(FText::FromString(TEXT("Pulse Rifle")))]
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(AmmoText, STextBlock).Text(FText::FromString(TEXT("30 / 120")))]
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(VehicleText, STextBlock).Text(FText::FromString(TEXT("Vehicle: Q | Abilities locked")))]
        ];
}

void UHeroHUDWidget::SetPlayerMode(const EHeroPlayerMode InMode)
{
    PlayerMode = InMode;
    RefreshModeText();
}

void UHeroHUDWidget::SetHealth(const float Health, const float MaxHealth)
{
    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("Health %.0f / %.0f"), Health, MaxHealth)));
    }
}

void UHeroHUDWidget::SetAmmo(const int32 AmmoInMagazine, const int32 ReserveAmmo)
{
    if (AmmoText)
    {
        AmmoText->SetText(FText::FromString(FString::Printf(TEXT("Ammo %d / %d"), AmmoInMagazine, ReserveAmmo)));
    }
}

void UHeroHUDWidget::SetReloading(const bool bReloading)
{
    if (AmmoText && bReloading)
    {
        AmmoText->SetText(FText::FromString(TEXT("Reloading...")));
    }
}

void UHeroHUDWidget::SetWeaponName(const FText& WeaponName)
{
    if (WeaponText)
    {
        WeaponText->SetText(WeaponName);
    }
}

void UHeroHUDWidget::SetVehicleHealth(const float Health, const float MaxHealth)
{
    if (VehicleText)
    {
        VehicleText->SetText(FText::FromString(FString::Printf(TEXT("Vehicle Health %.0f / %.0f | Ability 1 LOCKED | Ultimate NOT CONFIGURED"), Health, MaxHealth)));
    }
}

void UHeroHUDWidget::SetVehicleSpeed(const float SpeedKmh)
{
    if (VehicleText && PlayerMode == EHeroPlayerMode::Vehicle)
    {
        VehicleText->SetText(FText::FromString(FString::Printf(TEXT("Speed %.0f km/h | Vehicle weapon ready | Abilities locked"), SpeedKmh)));
    }
}

void UHeroHUDWidget::SetCrosshairVisible(const bool bVisible)
{
    if (CrosshairText)
    {
        CrosshairText->SetVisibility(bVisible ? EVisibility::Visible : EVisibility::Hidden);
    }
}

void UHeroHUDWidget::SetObjectiveStatus(const EHeroGameModeType GameMode, const EHeroTeam Team, const EHeroObjectiveState ObjectiveState, const float ObjectiveProgress01, const bool bContested)
{
    if (!ObjectiveText)
    {
        return;
    }

    const TCHAR* ModeName = GameMode == EHeroGameModeType::Escort ? TEXT("Escort") : GameMode == EHeroGameModeType::Control ? TEXT("Control") : TEXT("Sandbox");
    const TCHAR* TeamName = Team == EHeroTeam::TeamA ? TEXT("Team A") : Team == EHeroTeam::TeamB ? TEXT("Team B") : TEXT("No Team");
    const TCHAR* StateName = bContested || ObjectiveState == EHeroObjectiveState::Contested ? TEXT("CONTESTED") : TEXT("Objective Active");
    ObjectiveText->SetText(FText::FromString(FString::Printf(TEXT("%s | %s | %s | %.0f%%"), ModeName, TeamName, StateName, ObjectiveProgress01 * 100.0f)));
}

void UHeroHUDWidget::RefreshModeText()
{
    if (ModeText)
    {
        ModeText->SetText(PlayerMode == EHeroPlayerMode::Vehicle ? FText::FromString(TEXT("VEHICLE MODE - MECH ONLINE")) : FText::FromString(TEXT("HUMAN MODE")));
    }

    if (CrosshairText)
    {
        CrosshairText->SetText(PlayerMode == EHeroPlayerMode::Vehicle ? FText::FromString(TEXT("<+>")) : FText::FromString(TEXT("+")));
    }
}

#include "HeroHUDWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
    FSlateFontInfo HeroFont(const int32 Size, const FName Typeface = TEXT("Regular"))
    {
        return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
    }

// 기능: HUD 패널 배경색을 반환한다.
    FLinearColor PanelColor()
    {
        return FLinearColor(0.015f, 0.025f, 0.055f, 0.78f);
    }

// 기능: HUD 강조색을 반환한다.
    FLinearColor AccentColor()
    {
        return FLinearColor(0.0f, 0.72f, 1.0f, 1.0f);
    }
}

// 기능: 화면 중앙 조준선, 히트마커, 좌하단 체력, 우하단 무기 패널을 구성한다.
TSharedRef<SWidget> UHeroHUDWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        [
            SNew(SBorder)
            .Visibility(EVisibility::Collapsed)
            .Padding(FMargin(26.0f, 14.0f))
            .BorderBackgroundColor(FLinearColor(0.01f, 0.015f, 0.035f, 0.72f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                [SAssignNew(ModeText, STextBlock).Text(FText::FromString(TEXT(""))).Font(HeroFont(22, TEXT("Bold"))).ColorAndOpacity(AccentColor())]
                + SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                [SAssignNew(ObjectiveText, STextBlock).Text(FText::FromString(TEXT(""))).Font(HeroFont(20, TEXT("Bold"))).ColorAndOpacity(FLinearColor::White)]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                [SAssignNew(BotText, STextBlock).Text(FText::FromString(TEXT(""))).Font(HeroFont(18, TEXT("Bold"))).ColorAndOpacity(FLinearColor(0.9f, 0.95f, 1.0f, 1.0f))]
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SOverlay)
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SAssignNew(CrosshairText, STextBlock)
                .Text(FText::FromString(TEXT("+")))
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                .Font(HeroFont(38, TEXT("Bold")))
                .ColorAndOpacity(FLinearColor(0.76f, 1.0f, 0.82f, 0.98f))
                .ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
                .ShadowOffset(FVector2D(1.0f, 1.0f))
            ]
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.0f, 44.0f, 0.0f, 0.0f)
            [
                SAssignNew(HitMarkerText, STextBlock)
                .Text(FText::FromString(TEXT("HIT")))
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                .Font(HeroFont(18, TEXT("Bold")))
                .ColorAndOpacity(FLinearColor(1.0f, 0.92f, 0.35f, 1.0f))
                .Visibility(EVisibility::Hidden)
            ]
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.0f, 76.0f, 0.0f, 0.0f)
            [
                SAssignNew(DamageText, STextBlock)
                .Text(FText::FromString(TEXT("34")))
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                .Font(HeroFont(16, TEXT("Bold")))
                .ColorAndOpacity(FLinearColor(1.0f, 0.82f, 0.32f, 1.0f))
                .Visibility(EVisibility::Hidden)
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Bottom)
        .Padding(28.0f)
        [
            SNew(SBorder)
            .Padding(FMargin(18.0f, 14.0f))
// 기능: HUD 패널 배경색을 반환한다.
            .BorderBackgroundColor(PanelColor())
            [
                SNew(SVerticalBox)
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                + SVerticalBox::Slot().AutoHeight()[SAssignNew(HealthText, STextBlock).Text(FText::FromString(TEXT("HEALTH 200 / 200"))).Font(HeroFont(28, TEXT("Bold"))).ColorAndOpacity(FLinearColor::White)]
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f)[SAssignNew(VehicleText, STextBlock).Text(FText::FromString(TEXT("Q: Vehicle Mode | E: Interact | Ctrl/C: Crouch"))).Font(HeroFont(14)).ColorAndOpacity(FLinearColor(0.75f, 0.86f, 1.0f, 1.0f))]
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Bottom)
        .Padding(28.0f)
        [
            SNew(SBorder)
            .Padding(FMargin(18.0f, 14.0f))
// 기능: HUD 패널 배경색을 반환한다.
            .BorderBackgroundColor(PanelColor())
            [
                SNew(SVerticalBox)
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                + SVerticalBox::Slot().AutoHeight()[SAssignNew(WeaponText, STextBlock).Text(FText::FromString(TEXT("PULSE RIFLE"))).Font(HeroFont(19, TEXT("Bold"))).ColorAndOpacity(AccentColor())]
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)[SAssignNew(AmmoText, STextBlock).Text(FText::FromString(TEXT("30 / 120"))).Font(HeroFont(34, TEXT("Bold"))).ColorAndOpacity(FLinearColor::White)]
// 기능: HUD 텍스트에 사용할 기본 Slate 폰트를 만든다.
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)[SAssignNew(HeatText, STextBlock).Text(FText::FromString(TEXT("HEAT 0%"))).Font(HeroFont(13)).ColorAndOpacity(FLinearColor(0.74f, 0.84f, 0.95f, 1.0f))]
            ]
        ];
}

// 기능: 히트마커와 데미지 숫자의 표시 시간을 감소시키고 자동으로 숨긴다.
void UHeroHUDWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    HitMarkerTimer = FMath::Max(0.0f, HitMarkerTimer - InDeltaTime);
    DamageTextTimer = FMath::Max(0.0f, DamageTextTimer - InDeltaTime);

    if (HitMarkerText)
    {
        HitMarkerText->SetVisibility(HitMarkerTimer > 0.0f ? EVisibility::Visible : EVisibility::Hidden);
    }
    if (DamageText)
    {
        DamageText->SetVisibility(DamageTextTimer > 0.0f ? EVisibility::Visible : EVisibility::Hidden);
    }
}

// 기능: HUD의 영웅/차량 모드 표시 상태를 갱신한다.
void UHeroHUDWidget::SetPlayerMode(const EHeroPlayerMode InMode)
{
    PlayerMode = InMode;
    RefreshModeText();
}

// 기능: 체력 수치를 HUD에 표시한다.
void UHeroHUDWidget::SetHealth(const float Health, const float MaxHealth)
{
    if (HealthText)
    {
        const float Ratio = MaxHealth > KINDA_SMALL_NUMBER ? Health / MaxHealth : 0.0f;
        const TCHAR* State = Ratio <= 0.25f ? TEXT("CRITICAL") : TEXT("HEALTH");
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%s %.0f / %.0f"), State, Health, MaxHealth)));
    }
}

// 기능: 탄창/예비 탄약 수를 HUD에 표시한다.
void UHeroHUDWidget::SetAmmo(const int32 AmmoInMagazine, const int32 ReserveAmmo)
{
    if (AmmoText)
    {
        AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), AmmoInMagazine, ReserveAmmo)));
    }
}

// 기능: 재장전 중일 때 탄약 영역에 RELOADING을 표시한다.
void UHeroHUDWidget::SetReloading(const bool bReloading)
{
    if (AmmoText && bReloading)
    {
        AmmoText->SetText(FText::FromString(TEXT("RELOADING")));
    }
}

// 기능: 현재 무기 이름을 HUD에 표시한다.
void UHeroHUDWidget::SetWeaponName(const FText& WeaponName)
{
    if (WeaponText)
    {
        WeaponText->SetText(WeaponName);
    }
}

// 기능: 연사 열/확산 상태를 HUD에 표시한다.
void UHeroHUDWidget::SetWeaponHeat(const float Heat01)
{
    if (HeatText)
    {
        HeatText->SetText(FText::FromString(FString::Printf(TEXT("HEAT %.0f%%"), FMath::Clamp(Heat01, 0.0f, 1.0f) * 100.0f)));
    }
}

// 기능: 영웅 모드일 때 차량/방어구 상태 안내를 표시한다.
void UHeroHUDWidget::SetVehicleHealth(const float Health, const float MaxHealth)
{
    if (VehicleText && PlayerMode != EHeroPlayerMode::Vehicle)
    {
        VehicleText->SetText(FText::FromString(FString::Printf(TEXT("Q: Vehicle Mode | Armor %.0f / %.0f | Ctrl/C: Crouch"), Health, MaxHealth)));
    }
}

// 기능: 차량 모드일 때 차량 속도를 HUD에 표시한다.
void UHeroHUDWidget::SetVehicleSpeed(const float SpeedKmh)
{
    if (VehicleText && PlayerMode == EHeroPlayerMode::Vehicle)
    {
        VehicleText->SetText(FText::FromString(FString::Printf(TEXT("VEHICLE %.0f km/h | Cannon ready"), SpeedKmh)));
    }
}

// 기능: 조준선 표시 여부를 변경한다.
void UHeroHUDWidget::SetCrosshairVisible(const bool bVisible)
{
    if (CrosshairText)
    {
        CrosshairText->SetVisibility(bVisible ? EVisibility::Visible : EVisibility::Hidden);
    }
}

// 기능: 목표 상태 데이터를 받지만 현재 기본 HUD에서는 상단 배너를 숨긴 상태로 유지한다.
void UHeroHUDWidget::SetObjectiveStatus(const EHeroGameModeType GameMode, const EHeroTeam Team, const EHeroObjectiveState ObjectiveState, const float ObjectiveProgress01, const bool bContested)
{
    if (ObjectiveText)
    {
        ObjectiveText->SetText(FText::FromString(BuildObjectiveString(GameMode, Team, ObjectiveState, ObjectiveProgress01, bContested)));
    }
}

// 기능: 봇 생존 수 데이터를 받지만 현재 기본 HUD에서는 상단 배너를 숨긴 상태로 유지한다.
void UHeroHUDWidget::SetBotStatus(const int32 AliveBots, const int32 TotalBots)
{
    if (BotText)
    {
        BotText->SetText(FText::FromString(FString::Printf(TEXT("BOTS %d/%d"), AliveBots, TotalBots)));
    }
}

// 기능: 명중/처치 표시와 데미지 숫자를 일정 시간 표시한다.
void UHeroHUDWidget::NotifyHitMarker(const float DamageAmount, const bool bKilled)
{
    HitMarkerTimer = bKilled ? 0.55f : 0.28f;
    DamageTextTimer = 0.55f;

    if (HitMarkerText)
    {
        HitMarkerText->SetText(FText::FromString(bKilled ? TEXT("ELIMINATED") : TEXT("HIT")));
    }
    if (DamageText)
    {
        DamageText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), DamageAmount)));
    }
}

// 기능: 플레이어 모드에 맞춰 내부 모드 텍스트와 조준선 모양을 갱신한다.
void UHeroHUDWidget::RefreshModeText()
{
    if (ModeText)
    {
        ModeText->SetText(PlayerMode == EHeroPlayerMode::Vehicle ? FText::FromString(TEXT("VEHICLE MODE")) : FText::FromString(TEXT("HERO MODE")));
    }

    if (CrosshairText)
    {
        CrosshairText->SetText(PlayerMode == EHeroPlayerMode::Vehicle ? FText::FromString(TEXT("<+>")) : FText::FromString(TEXT("+")));
    }
}

// 기능: 목표 상태 배너용 문자열을 조립한다.
FString UHeroHUDWidget::BuildObjectiveString(const EHeroGameModeType GameMode, const EHeroTeam Team, const EHeroObjectiveState ObjectiveState, const float ObjectiveProgress01, const bool bContested) const
{
    const TCHAR* ModeName = GameMode == EHeroGameModeType::Escort ? TEXT("ESCORT") : GameMode == EHeroGameModeType::Control ? TEXT("CONTROL") : GameMode == EHeroGameModeType::TeamDeathmatch ? TEXT("TDM") : TEXT("");
    const TCHAR* TeamName = Team == EHeroTeam::TeamA ? TEXT("TEAM A") : Team == EHeroTeam::TeamB ? TEXT("TEAM B") : TEXT("");
    const TCHAR* StateName = bContested || ObjectiveState == EHeroObjectiveState::Contested ? TEXT("CONTESTED") : ObjectiveState == EHeroObjectiveState::Captured ? TEXT("CAPTURED") : ObjectiveState == EHeroObjectiveState::Completed ? TEXT("COMPLETE") : ObjectiveState == EHeroObjectiveState::Active ? TEXT("ACTIVE") : TEXT("");
    return FString::Printf(TEXT("%s  |  %s  |  %s  |  %.0f%%"), ModeName, TeamName, StateName, FMath::Clamp(ObjectiveProgress01, 0.0f, 1.0f) * 100.0f);
}


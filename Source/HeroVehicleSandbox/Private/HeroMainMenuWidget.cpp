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
                [SNew(STextBlock).Text(FText::FromString(TEXT("HERO SANDBOX"))).Font(MenuFont(26, TEXT("Bold"))).ColorAndOpacity(FLinearColor(0.0f, 0.75f, 1.0f, 1.0f))]
                + SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right).VAlign(VAlign_Center)
                [SNew(STextBlock).Text(FText::FromString(TEXT("v0.1.2  |  ESC Settings"))).Font(MenuFont(15)).ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.98f, 1.0f))]
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Center)
        .Padding(72.0f, 90.0f, 0.0f, 48.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
            [SNew(STextBlock).Text(FText::FromString(TEXT("플레이"))).Font(MenuFont(52, TEXT("Bold"))).ColorAndOpacity(FLinearColor::White)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 28.0f)
            [SAssignNew(StatusText, STextBlock).Text(FText::FromString(TEXT("샌드박스, 전투 테스트, 사용자 지정 게임을 선택하십시오."))).Font(MenuFont(17)).ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.98f, 1.0f))]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).OnClicked_UObject(this, &UHeroMainMenuWidget::HandlePlaySandboxClicked)[SNew(STextBlock).Text(FText::FromString(TEXT("플레이 샌드박스"))).Font(MenuFont(24, TEXT("Bold")))] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCombatTestClicked)[SNew(STextBlock).Text(FText::FromString(TEXT("전투 테스트"))).Font(MenuFont(24, TEXT("Bold")))] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCustomGameClicked)[SNew(STextBlock).Text(FText::FromString(TEXT("사용자 지정 게임"))).Font(MenuFont(24, TEXT("Bold")))] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 22.0f, 0.0f, 6.0f)
            [SNew(SButton).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleSettingsClicked)[SNew(STextBlock).Text(FText::FromString(TEXT("설정"))).Font(MenuFont(22))] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCreditsClicked)[SNew(STextBlock).Text(FText::FromString(TEXT("패치 노트"))).Font(MenuFont(22))] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleQuitClicked)[SNew(STextBlock).Text(FText::FromString(TEXT("게임 종료"))).Font(MenuFont(22))] ]
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
                + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("0.1.2 목표"))).Font(MenuFont(27, TEXT("Bold"))).ColorAndOpacity(FLinearColor(0.0f, 0.72f, 1.0f, 1.0f))]
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 0.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("• 1인칭 이동/시점 보강\n• Ctrl/C 앉기\n• 공중 조작 약화\n• 사격 반동/히트마커\n• 전투 테스트 봇 스폰\n• ESC 설정 메뉴"))).Font(MenuFont(17)).ColorAndOpacity(FLinearColor::White)]
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
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("0.1.2: 앉기, 공중 제어, 전투 테스트 봇, 히트마커, UI 정리 패치.")));
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


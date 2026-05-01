#include "HeroMainMenuWidget.h"

#include "HeroPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UHeroMainMenuWidget::RebuildWidget()
{
    return SNew(SBorder)
        .Padding(48.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 24.0f)
            [SNew(STextBlock).Text(FText::FromString(TEXT("HeroVehicleSandbox")))]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Play Sandbox"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandlePlaySandboxClicked)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Combat Test"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCombatTestClicked)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Settings"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleSettingsClicked)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Credits"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCreditsClicked)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Quit"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleQuitClicked)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 18.0f, 0.0f, 0.0f)
            [SAssignNew(StatusText, STextBlock).Text(FText::FromString(TEXT("Play Sandbox starts the first-person hero test.")))]
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
        PC->StartSandboxFromMenu();
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
        StatusText->SetText(FText::FromString(TEXT("HeroVehicleSandbox foundation: RaceCore vehicle baseline plus hero FPS sandbox.")));
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

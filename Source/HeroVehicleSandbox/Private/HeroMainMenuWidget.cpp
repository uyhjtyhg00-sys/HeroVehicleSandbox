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
            [SNew(SButton).Text(FText::FromString(TEXT("Combat Test"))).IsEnabled(false)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Settings"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleSettingsClicked)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Credits"))).IsEnabled(false)]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [SNew(SButton).Text(FText::FromString(TEXT("Quit"))).OnClicked_UObject(this, &UHeroMainMenuWidget::HandleQuitClicked)]
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

FReply UHeroMainMenuWidget::HandleSettingsClicked()
{
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->ShowSettingsMenu();
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

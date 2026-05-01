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
    // Function: Build main menu without showing internal development goals as player-facing HUD text.
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
                .Text(FText::FromString(TEXT("PLAY")))
                .Font(MenuFont(52, TEXT("Bold")))
                .ColorAndOpacity(FLinearColor::White)
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 28.0f)
            [
                SAssignNew(StatusText, STextBlock)
                .Text(FText::FromString(TEXT("Select a mode.")))
                .Font(MenuFont(17))
                .ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.98f, 1.0f))
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandlePlaySandboxClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Play Sandbox"))).Font(MenuFont(24, TEXT("Bold")))]
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCombatTestClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Combat Test"))).Font(MenuFont(24, TEXT("Bold")))]
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCustomGameClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Custom Game"))).Font(MenuFont(24, TEXT("Bold")))]
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 22.0f, 0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleSettingsClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Settings"))).Font(MenuFont(22))]
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleCreditsClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Patch Notes"))).Font(MenuFont(22))]
            ]

            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f)
            [
                SNew(SButton)
                .OnClicked_UObject(this, &UHeroMainMenuWidget::HandleQuitClicked)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Quit"))).Font(MenuFont(22))]
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
                    .Text(FText::FromString(TEXT("Press Patch Notes to view the current release notes.")))
                    .Font(MenuFont(17))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ];
}

FReply UHeroMainMenuWidget::HandlePlaySandboxClicked()
{
    // Function: Start the basic sandbox flow.
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->StartSandboxFromMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleCombatTestClicked()
{
    // Function: Start combat test flow with bots/objective setup.
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->StartCombatTestFromMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleCustomGameClicked()
{
    // Function: Start the custom game flow.
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->StartCustomGameFromMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleSettingsClicked()
{
    // Function: Open settings from the main menu.
    if (AHeroPlayerController* PC = GetOwningPlayer<AHeroPlayerController>())
    {
        PC->ShowSettingsMenu();
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleCreditsClicked()
{
    // Function: Patch notes button displays real patch notes instead of a placeholder.
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("v0.1.2 Patch Notes")));
    }
    if (PatchNotesText)
    {
        PatchNotesText->SetText(ReleaseNotes012());
    }
    return FReply::Handled();
}

FReply UHeroMainMenuWidget::HandleQuitClicked()
{
    // Function: Quit the game from the main menu.
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
    return FReply::Handled();
}

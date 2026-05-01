#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroMainMenuWidget.generated.h"

UCLASS()
class HEROVEHICLESANDBOX_API UHeroMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    FReply HandlePlaySandboxClicked();
    FReply HandleCombatTestClicked();
    FReply HandleCustomGameClicked();
    FReply HandleSettingsClicked();
    FReply HandleCreditsClicked();
    FReply HandleQuitClicked();

private:
    TSharedPtr<class STextBlock> StatusText;
};

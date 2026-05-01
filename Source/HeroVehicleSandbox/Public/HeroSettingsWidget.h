#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroTypes.h"
#include "HeroSettingsWidget.generated.h"

UCLASS()
class HEROVEHICLESANDBOX_API UHeroSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    void SetMouseSensitivity(float NewSensitivity);

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    float GetMouseSensitivity() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    void SetFov(float NewFov);

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    float GetFov() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    void ApplySettings();

    UFUNCTION(BlueprintCallable, Category="Hero|Settings")
    void ResetToDefaults();

private:
    void LoadSettings();
    FReply HandleApplyClicked();
    FReply HandleResetClicked();
    FReply HandleBackClicked();

private:
    FHeroAimSettings WorkingAimSettings;
};

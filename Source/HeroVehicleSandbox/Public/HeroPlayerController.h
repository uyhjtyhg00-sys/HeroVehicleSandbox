#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HeroPlayerController.generated.h"

class UHeroHUDWidget;
class UHeroMainMenuWidget;
class UHeroSettingsWidget;
class UUserWidget;
class AHeroCharacter;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHeroPlayerController();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Hero|UI")
    void ShowMainMenu();

    UFUNCTION(BlueprintCallable, Category="Hero|UI")
    void ShowHUD();

    UFUNCTION(BlueprintCallable, Category="Hero|UI")
    void ShowSettingsMenu();

    UFUNCTION(BlueprintCallable, Category="Hero|UI")
    void StartSandboxFromMenu();

    UFUNCTION(BlueprintCallable, Category="Hero|UI")
    UHeroHUDWidget* GetHeroHUD() const;

private:
    void ClearMenu();
    void EnsureHeroPawnPossessed();
    void SetMenuInputMode();
    void SetGameInputMode();
    void UpdateHUDFromPawn();

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero|Player", meta=(AllowPrivateAccess="true"))
    TSubclassOf<AHeroCharacter> HeroCharacterClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero|UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UHeroHUDWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero|UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UHeroMainMenuWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero|UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UHeroSettingsWidget> SettingsWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> CurrentMenu;

    UPROPERTY(Transient)
    TObjectPtr<UHeroHUDWidget> HeroHUD;
};

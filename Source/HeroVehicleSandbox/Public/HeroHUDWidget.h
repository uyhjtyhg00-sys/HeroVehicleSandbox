#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroTypes.h"
#include "HeroHUDWidget.generated.h"

class STextBlock;

UCLASS()
class HEROVEHICLESANDBOX_API UHeroHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetPlayerMode(EHeroPlayerMode InMode);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetHealth(float Health, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetAmmo(int32 AmmoInMagazine, int32 ReserveAmmo);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetReloading(bool bReloading);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetWeaponName(const FText& WeaponName);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetWeaponHeat(float Heat01);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetVehicleHealth(float Health, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetVehicleSpeed(float SpeedKmh);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetCrosshairVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetObjectiveStatus(EHeroGameModeType GameMode, EHeroTeam Team, EHeroObjectiveState ObjectiveState, float ObjectiveProgress01, bool bContested);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void SetBotStatus(int32 AliveBots, int32 TotalBots);

    UFUNCTION(BlueprintCallable, Category="Hero|HUD")
    void NotifyHitMarker(float DamageAmount, bool bKilled);

private:
    void RefreshModeText();
    FString BuildObjectiveString(EHeroGameModeType GameMode, EHeroTeam Team, EHeroObjectiveState ObjectiveState, float ObjectiveProgress01, bool bContested) const;

private:
    EHeroPlayerMode PlayerMode = EHeroPlayerMode::Human;
    TSharedPtr<STextBlock> CrosshairText;
    TSharedPtr<STextBlock> HitMarkerText;
    TSharedPtr<STextBlock> DamageText;
    TSharedPtr<STextBlock> HealthText;
    TSharedPtr<STextBlock> AmmoText;
    TSharedPtr<STextBlock> ModeText;
    TSharedPtr<STextBlock> WeaponText;
    TSharedPtr<STextBlock> ObjectiveText;
    TSharedPtr<STextBlock> VehicleText;
    TSharedPtr<STextBlock> BotText;
    TSharedPtr<STextBlock> HeatText;
    float HitMarkerTimer = 0.0f;
    float DamageTextTimer = 0.0f;
};

#pragma once

#include "CoreMinimal.h"
#include "HeroTypes.h"

class HEROVEHICLESANDBOX_API FHeroWeaponCore
{
public:
    void Initialize(const FHeroWeaponConfig& InConfig);
    void Tick(float DeltaSeconds);

    bool CanFire() const;
    bool ConsumeShot();
    bool StartReload();
    void FinishReload();

    const FHeroWeaponConfig& GetConfig() const;
    int32 GetAmmoInMagazine() const;
    int32 GetReserveAmmo() const;
    bool IsReloading() const;
    float GetReloadRemainingSeconds() const;

private:
    FHeroWeaponConfig Config;
    int32 AmmoInMagazine = 0;
    int32 ReserveAmmo = 0;
    bool bReloading = false;
    float ReloadRemainingSeconds = 0.0f;
};

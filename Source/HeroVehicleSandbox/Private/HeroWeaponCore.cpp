#include "HeroWeaponCore.h"

void FHeroWeaponCore::Initialize(const FHeroWeaponConfig& InConfig)
{
    Config = InConfig;
    Config.MagazineSize = FMath::Max(1, Config.MagazineSize);
    Config.ReserveAmmo = FMath::Max(0, Config.ReserveAmmo);
    Config.FireRateRoundsPerMinute = FMath::Max(1.0f, Config.FireRateRoundsPerMinute);
    AmmoInMagazine = Config.MagazineSize;
    ReserveAmmo = Config.ReserveAmmo;
    bReloading = false;
    ReloadRemainingSeconds = 0.0f;
}

void FHeroWeaponCore::Tick(const float DeltaSeconds)
{
    if (!bReloading)
    {
        return;
    }

    ReloadRemainingSeconds = FMath::Max(0.0f, ReloadRemainingSeconds - DeltaSeconds);
    if (ReloadRemainingSeconds <= 0.0f)
    {
        FinishReload();
    }
}

bool FHeroWeaponCore::CanFire() const
{
    return !bReloading && AmmoInMagazine > 0;
}

bool FHeroWeaponCore::ConsumeShot()
{
    if (!CanFire())
    {
        return false;
    }

    --AmmoInMagazine;
    return true;
}

bool FHeroWeaponCore::StartReload()
{
    if (bReloading || AmmoInMagazine >= Config.MagazineSize || ReserveAmmo <= 0)
    {
        return false;
    }

    bReloading = true;
    ReloadRemainingSeconds = FMath::Max(0.01f, Config.ReloadSeconds);
    return true;
}

void FHeroWeaponCore::FinishReload()
{
    const int32 NeededAmmo = Config.MagazineSize - AmmoInMagazine;
    const int32 AmmoToLoad = FMath::Min(NeededAmmo, ReserveAmmo);
    AmmoInMagazine += AmmoToLoad;
    ReserveAmmo -= AmmoToLoad;
    bReloading = false;
    ReloadRemainingSeconds = 0.0f;
}

const FHeroWeaponConfig& FHeroWeaponCore::GetConfig() const
{
    return Config;
}

int32 FHeroWeaponCore::GetAmmoInMagazine() const
{
    return AmmoInMagazine;
}

int32 FHeroWeaponCore::GetReserveAmmo() const
{
    return ReserveAmmo;
}

bool FHeroWeaponCore::IsReloading() const
{
    return bReloading;
}

float FHeroWeaponCore::GetReloadRemainingSeconds() const
{
    return ReloadRemainingSeconds;
}

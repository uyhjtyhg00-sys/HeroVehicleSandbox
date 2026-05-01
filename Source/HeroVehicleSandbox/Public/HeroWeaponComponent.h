#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeroTypes.h"
#include "HeroWeaponCore.h"
#include "HeroWeaponComponent.generated.h"

class AHeroProjectile;
class USceneComponent;

UCLASS(ClassGroup=(Hero), meta=(BlueprintSpawnableComponent))
class HEROVEHICLESANDBOX_API UHeroWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeroWeaponComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    void SetPlayerMode(EHeroPlayerMode NewMode);

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    void StartFire(USceneComponent* FireOrigin = nullptr);

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    void Reload();

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    void SetAiming(bool bNewAiming);

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    int32 GetAmmoInMagazine() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    int32 GetReserveAmmo() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    bool IsReloading() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    bool IsAiming() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    FText GetWeaponName() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    float GetWeaponHeat01() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Weapon")
    const FHeroWeaponConfig& GetCurrentWeaponConfig() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapons")
    FHeroWeaponConfig HumanPrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapons")
    FHeroWeaponConfig VehiclePrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle")
    TArray<FHeroAbilitySlot> VehicleAbilitySlots;

private:
    FHeroWeaponCore& GetMutableCore();
    const FHeroWeaponCore& GetCore() const;
    void TryFire(USceneComponent* FireOrigin);
    void FireHitScan(const FVector& Origin, const FVector& Direction, const FHeroWeaponConfig& Config);
    void FireProjectile(const FVector& Origin, const FVector& Direction, const FHeroWeaponConfig& Config);
    bool GetFireView(USceneComponent* FireOrigin, FVector& OutOrigin, FVector& OutDirection) const;
    FVector ApplySpread(const FVector& Direction, const FHeroWeaponConfig& Config) const;
    void ApplyRecoil(const FHeroWeaponConfig& Config);
    void NotifyLocalHit(float DamageAmount, bool bKilled);

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon", meta=(AllowPrivateAccess="true"))
    TSubclassOf<AHeroProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Weapon", meta=(AllowPrivateAccess="true"))
    bool bDrawDebugFireTrace = false;

    EHeroPlayerMode PlayerMode = EHeroPlayerMode::Human;
    FHeroWeaponCore HumanWeaponCore;
    FHeroWeaponCore VehicleWeaponCore;
    bool bWantsToFire = false;
    bool bAiming = false;
    float FireCooldownSeconds = 0.0f;
    float WeaponHeat01 = 0.0f;

    UPROPERTY(Transient)
    TObjectPtr<USceneComponent> CachedFireOrigin;
};

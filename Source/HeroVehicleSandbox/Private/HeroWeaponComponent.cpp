#include "HeroWeaponComponent.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HeroHealthComponent.h"
#include "HeroProjectile.h"

UHeroWeaponComponent::UHeroWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    HumanPrimaryWeapon.WeaponName = FText::FromString(TEXT("Pulse Rifle"));
    HumanPrimaryWeapon.bIsHitScan = true;
    HumanPrimaryWeapon.FireMode = EHeroWeaponFireMode::HitScan;
    HumanPrimaryWeapon.Damage = 34.0f;
    HumanPrimaryWeapon.FireRateRoundsPerMinute = 620.0f;
    HumanPrimaryWeapon.MagazineSize = 30;
    HumanPrimaryWeapon.ReserveAmmo = 120;

    VehiclePrimaryWeapon.WeaponName = FText::FromString(TEXT("Vehicle Cannon"));
    VehiclePrimaryWeapon.bIsHitScan = true;
    VehiclePrimaryWeapon.FireMode = EHeroWeaponFireMode::HitScan;
    VehiclePrimaryWeapon.Damage = 22.0f;
    VehiclePrimaryWeapon.FireRateRoundsPerMinute = 720.0f;
    VehiclePrimaryWeapon.SpreadDegrees = 0.8f;
    VehiclePrimaryWeapon.RangeMeters = 180.0f;
    VehiclePrimaryWeapon.MagazineSize = 60;
    VehiclePrimaryWeapon.ReserveAmmo = 240;
    VehiclePrimaryWeapon.ReloadSeconds = 2.2f;
    VehiclePrimaryWeapon.ProjectileSpeedMetersPerSecond = 80.0f;
    VehiclePrimaryWeapon.ExplosionRadiusMeters = 2.5f;

    VehicleAbilitySlots.SetNum(3);
}

void UHeroWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    HumanWeaponCore.Initialize(HumanPrimaryWeapon);
    VehicleWeaponCore.Initialize(VehiclePrimaryWeapon);
}

void UHeroWeaponComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    HumanWeaponCore.Tick(DeltaTime);
    VehicleWeaponCore.Tick(DeltaTime);
    FireCooldownSeconds = FMath::Max(0.0f, FireCooldownSeconds - DeltaTime);

    if (bWantsToFire)
    {
        TryFire(CachedFireOrigin.Get());
    }
}

void UHeroWeaponComponent::SetPlayerMode(const EHeroPlayerMode NewMode)
{
    PlayerMode = NewMode;
    FireCooldownSeconds = 0.0f;
}

void UHeroWeaponComponent::StartFire(USceneComponent* FireOrigin)
{
    CachedFireOrigin = FireOrigin;
    bWantsToFire = true;
    TryFire(FireOrigin);
}

void UHeroWeaponComponent::StopFire()
{
    bWantsToFire = false;
}

void UHeroWeaponComponent::Reload()
{
    GetMutableCore().StartReload();
}

void UHeroWeaponComponent::SetAiming(const bool bNewAiming)
{
    bAiming = bNewAiming;
}

int32 UHeroWeaponComponent::GetAmmoInMagazine() const
{
    return GetCore().GetAmmoInMagazine();
}

int32 UHeroWeaponComponent::GetReserveAmmo() const
{
    return GetCore().GetReserveAmmo();
}

bool UHeroWeaponComponent::IsReloading() const
{
    return GetCore().IsReloading();
}

bool UHeroWeaponComponent::IsAiming() const
{
    return bAiming;
}

FText UHeroWeaponComponent::GetWeaponName() const
{
    return GetCore().GetConfig().WeaponName;
}

const FHeroWeaponConfig& UHeroWeaponComponent::GetCurrentWeaponConfig() const
{
    return GetCore().GetConfig();
}

FHeroWeaponCore& UHeroWeaponComponent::GetMutableCore()
{
    return PlayerMode == EHeroPlayerMode::Vehicle ? VehicleWeaponCore : HumanWeaponCore;
}

const FHeroWeaponCore& UHeroWeaponComponent::GetCore() const
{
    return PlayerMode == EHeroPlayerMode::Vehicle ? VehicleWeaponCore : HumanWeaponCore;
}

void UHeroWeaponComponent::TryFire(USceneComponent* FireOrigin)
{
    FHeroWeaponCore& Core = GetMutableCore();
    if (FireCooldownSeconds > 0.0f || !Core.CanFire())
    {
        if (Core.GetAmmoInMagazine() <= 0)
        {
            Core.StartReload();
        }
        return;
    }

    FVector Origin;
    FVector Direction;
    if (!GetFireView(FireOrigin, Origin, Direction))
    {
        return;
    }

    if (!Core.ConsumeShot())
    {
        return;
    }

    const FHeroWeaponConfig& Config = Core.GetConfig();
    Direction = ApplySpread(Direction, Config);

    if (Config.bIsHitScan || Config.FireMode == EHeroWeaponFireMode::HitScan)
    {
        FireHitScan(Origin, Direction, Config);
    }
    else
    {
        FireProjectile(Origin, Direction, Config);
    }

    FireCooldownSeconds = 60.0f / FMath::Max(1.0f, Config.FireRateRoundsPerMinute);
}

void UHeroWeaponComponent::FireHitScan(const FVector& Origin, const FVector& Direction, const FHeroWeaponConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HeroHitScan), true, GetOwner());
    const FVector End = Origin + Direction * Config.RangeMeters * 100.0f;
    FHitResult Hit;

    if (World->LineTraceSingleByChannel(Hit, Origin, End, ECC_Visibility, QueryParams))
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (UHeroHealthComponent* Health = HitActor->FindComponentByClass<UHeroHealthComponent>())
            {
                Health->ApplyDamage(Config.Damage, GetOwner());
            }
        }
    }
}

void UHeroWeaponComponent::FireProjectile(const FVector& Origin, const FVector& Direction, const FHeroWeaponConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UClass* SpawnClass = ProjectileClass ? ProjectileClass.Get() : AHeroProjectile::StaticClass();
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AHeroProjectile* Projectile = World->SpawnActor<AHeroProjectile>(SpawnClass, Origin + Direction * 80.0f, Direction.Rotation(), SpawnParams);
    if (Projectile)
    {
        Projectile->ConfigureProjectile(Config.Damage, Config.ExplosionRadiusMeters, 1200.0f, GetOwner());
        Projectile->SetLaunchVelocity(Direction * Config.ProjectileSpeedMetersPerSecond * 100.0f);
    }
}

bool UHeroWeaponComponent::GetFireView(USceneComponent* FireOrigin, FVector& OutOrigin, FVector& OutDirection) const
{
    if (FireOrigin)
    {
        OutOrigin = FireOrigin->GetComponentLocation();
        OutDirection = FireOrigin->GetForwardVector();
        return true;
    }

    if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
    {
        if (const AController* Controller = PawnOwner->GetController())
        {
            FRotator ViewRotation;
            Controller->GetPlayerViewPoint(OutOrigin, ViewRotation);
            OutDirection = ViewRotation.Vector();
            return true;
        }
    }

    if (const AActor* Owner = GetOwner())
    {
        OutOrigin = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 80.0f;
        OutDirection = Owner->GetActorForwardVector();
        return true;
    }

    return false;
}

FVector UHeroWeaponComponent::ApplySpread(const FVector& Direction, const FHeroWeaponConfig& Config) const
{
    const float SpreadDegrees = Config.SpreadDegrees * (bAiming ? Config.AimSpreadMultiplier : 1.0f);
    if (SpreadDegrees <= KINDA_SMALL_NUMBER)
    {
        return Direction.GetSafeNormal();
    }

    return FMath::VRandCone(Direction.GetSafeNormal(), FMath::DegreesToRadians(SpreadDegrees));
}

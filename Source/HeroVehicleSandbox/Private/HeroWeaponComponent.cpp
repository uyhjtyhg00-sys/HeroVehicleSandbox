#include "HeroWeaponComponent.h"
#include "HeroCharacter.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HeroHealthComponent.h"
#include "HeroHUDWidget.h"
#include "HeroPlayerController.h"
#include "HeroProjectile.h"

// 기능: 인간/차량 기본 무기 설정과 컴포넌트 tick을 초기화한다.
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
    HumanPrimaryWeapon.SpreadDegrees = 0.48f;
    HumanPrimaryWeapon.AimSpreadMultiplier = 0.30f;

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

// 기능: 무기 코어를 현재 설정값으로 초기화한다.
void UHeroWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    HumanWeaponCore.Initialize(HumanPrimaryWeapon);
    VehicleWeaponCore.Initialize(VehiclePrimaryWeapon);
}

// 기능: 무기 쿨다운, 재장전, 연사 상태, 열 누적을 매 프레임 갱신한다.
void UHeroWeaponComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    HumanWeaponCore.Tick(DeltaTime);
    VehicleWeaponCore.Tick(DeltaTime);
    FireCooldownSeconds = FMath::Max(0.0f, FireCooldownSeconds - DeltaTime);
    WeaponHeat01 = FMath::Max(0.0f, WeaponHeat01 - DeltaTime * 1.65f);

    if (bWantsToFire)
    {
        TryFire(CachedFireOrigin.Get());
    }
}

// 기능: 영웅/차량 모드에 따라 활성 무기 코어를 전환한다.
void UHeroWeaponComponent::SetPlayerMode(const EHeroPlayerMode NewMode)
{
    PlayerMode = NewMode;
    FireCooldownSeconds = 0.0f;
}

// 기능: 발사 입력을 시작하고 가능한 경우 즉시 첫 발을 발사한다.
void UHeroWeaponComponent::StartFire(USceneComponent* FireOrigin)
{
    CachedFireOrigin = FireOrigin;
    bWantsToFire = true;
    TryFire(FireOrigin);
}

// 기능: 발사 입력을 중지한다.
void UHeroWeaponComponent::StopFire()
{
    bWantsToFire = false;
}

// 기능: 현재 무기 코어의 재장전을 요청한다.
void UHeroWeaponComponent::Reload()
{
    GetMutableCore().StartReload();
}

// 기능: 조준 상태를 설정해 확산/FOV 계산에 반영한다.
void UHeroWeaponComponent::SetAiming(const bool bNewAiming)
{
    bAiming = bNewAiming;
}

// 기능: 현재 탄창 내 탄약 수를 반환한다.
int32 UHeroWeaponComponent::GetAmmoInMagazine() const
{
    return GetCore().GetAmmoInMagazine();
}

// 기능: 현재 예비 탄약 수를 반환한다.
int32 UHeroWeaponComponent::GetReserveAmmo() const
{
    return GetCore().GetReserveAmmo();
}

// 기능: 현재 재장전 중인지 반환한다.
bool UHeroWeaponComponent::IsReloading() const
{
    return GetCore().IsReloading();
}

// 기능: 현재 우클릭 조준 상태인지 반환한다.
bool UHeroWeaponComponent::IsAiming() const
{
    return bAiming;
}

// 기능: 현재 활성 무기의 이름을 반환한다.
FText UHeroWeaponComponent::GetWeaponName() const
{
    return GetCore().GetConfig().WeaponName;
}

// 기능: 현재 무기 열/확산 상태를 0~1 값으로 반환한다.
float UHeroWeaponComponent::GetWeaponHeat01() const
{
    return WeaponHeat01;
}

// 기능: 현재 활성 무기 코어의 설정 구조체를 반환한다.
const FHeroWeaponConfig& UHeroWeaponComponent::GetCurrentWeaponConfig() const
{
    return GetCore().GetConfig();
}

// 기능: 현재 플레이어 모드에 맞는 수정 가능한 무기 코어를 반환한다.
FHeroWeaponCore& UHeroWeaponComponent::GetMutableCore()
{
    return PlayerMode == EHeroPlayerMode::Vehicle ? VehicleWeaponCore : HumanWeaponCore;
}

// 기능: 현재 플레이어 모드에 맞는 읽기 전용 무기 코어를 반환한다.
const FHeroWeaponCore& UHeroWeaponComponent::GetCore() const
{
    return PlayerMode == EHeroPlayerMode::Vehicle ? VehicleWeaponCore : HumanWeaponCore;
}

// 기능: 쿨다운과 탄약을 검사한 뒤 히트스캔 또는 투사체 발사를 실행한다.
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

    ApplyRecoil(Config);
    WeaponHeat01 = FMath::Clamp(WeaponHeat01 + 0.14f, 0.0f, 1.0f);
    FireCooldownSeconds = 60.0f / FMath::Max(1.0f, Config.FireRateRoundsPerMinute);
}

// 기능: 카메라 기준 히트스캔 사격과 데미지 처리를 수행한다.
// 기능: 카메라 기준 히트스캔 사격과 데미지 적용을 처리한다.
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

    const bool bHit = World->LineTraceSingleByChannel(Hit, Origin, End, ECC_Visibility, QueryParams);
    if (bDrawDebugFireTrace)
    {
        DrawDebugLine(World, Origin, bHit ? Hit.ImpactPoint : End, bHit ? FColor::Green : FColor::Cyan, false, 0.08f, 0, 1.5f);
    }

    if (bHit)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (UHeroHealthComponent* Health = HitActor->FindComponentByClass<UHeroHealthComponent>())
            {
                const float AppliedDamage = Health->ApplyDamage(Config.Damage, GetOwner());
                if (AppliedDamage > 0.0f)
                {
                    NotifyLocalHit(AppliedDamage, Health->IsDead());
                }
            }
        }
    }
}

// 기능: 투사체 무기를 스폰하고 초기 속도를 부여한다.
// 기능: 투사체 액터를 생성하고 발사 방향/속도를 설정한다.
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

// 기능: 조준/열 상태에 따른 탄퍼짐을 방향 벡터에 적용한다.
FVector UHeroWeaponComponent::ApplySpread(const FVector& Direction, const FHeroWeaponConfig& Config) const
{
    const float HeatSpread = WeaponHeat01 * 0.28f;
    const float SpreadDegrees = (Config.SpreadDegrees + HeatSpread) * (bAiming ? Config.AimSpreadMultiplier : 1.0f);
    if (SpreadDegrees <= KINDA_SMALL_NUMBER)
    {
        return Direction.GetSafeNormal();
    }

    return FMath::VRandCone(Direction.GetSafeNormal(), FMath::DegreesToRadians(SpreadDegrees));
}

// 기능: 마우스 반전 옵션과 무관하게 항상 위쪽으로 총기 반동을 적용한다.
void UHeroWeaponComponent::ApplyRecoil(const FHeroWeaponConfig& Config)
{
    AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
    if (!Hero || !Hero->IsPlayerControlled())
    {
        return;
    }

    const float ModeScale = PlayerMode == EHeroPlayerMode::Vehicle ? 0.55f : 1.0f;
    const float AimScale = bAiming ? 0.48f : 1.0f;
    const float PitchKick = 0.075f * ModeScale * AimScale * FMath::Clamp(Config.FireRateRoundsPerMinute / 600.0f, 0.65f, 1.35f);
    const float YawKick = FMath::FRandRange(-0.035f, 0.035f) * ModeScale * AimScale;

    Hero->ApplyViewKick(PitchKick, YawKick);
}

// 기능: 로컬 플레이어 HUD에 히트마커와 데미지 숫자를 표시한다.
// 기능: 로컬 HUD에 명중/처치 피드백을 전달한다.
void UHeroWeaponComponent::NotifyLocalHit(const float DamageAmount, const bool bKilled)
{
    const APawn* PawnOwner = Cast<APawn>(GetOwner());
    AHeroPlayerController* PC = PawnOwner ? Cast<AHeroPlayerController>(PawnOwner->GetController()) : nullptr;
    if (PC && PC->GetHeroHUD())
    {
        PC->GetHeroHUD()->NotifyHitMarker(DamageAmount, bKilled);
    }
}

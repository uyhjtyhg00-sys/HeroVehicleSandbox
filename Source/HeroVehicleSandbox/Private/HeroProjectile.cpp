#include "HeroProjectile.h"

#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HeroHealthComponent.h"

AHeroProjectile::AHeroProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    Collision->SetSphereRadius(12.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Block);
    Collision->SetNotifyRigidBodyCollision(true);
    SetRootComponent(Collision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 5500.0f;
    ProjectileMovement->MaxSpeed = 9000.0f;
    ProjectileMovement->ProjectileGravityScale = 0.2f;
    ProjectileMovement->bRotationFollowsVelocity = true;
}

void AHeroProjectile::BeginPlay()
{
    Super::BeginPlay();
    Collision->OnComponentHit.AddDynamic(this, &AHeroProjectile::HandleHit);
    SetLifeSpan(LifeSeconds);
}

void AHeroProjectile::ConfigureProjectile(const float InDamage, const float InExplosionRadiusMeters, const float InImpactImpulse, AActor* InInstigatorActor)
{
    Damage = FMath::Max(0.0f, InDamage);
    ExplosionRadiusMeters = FMath::Max(0.0f, InExplosionRadiusMeters);
    ImpactImpulse = FMath::Max(0.0f, InImpactImpulse);
    InstigatorActor = InInstigatorActor;
}

void AHeroProjectile::SetLaunchVelocity(const FVector& VelocityCmPerSecond)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = VelocityCmPerSecond;
    }
}

void AHeroProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    (void)HitComponent;
    (void)OtherComp;
    (void)NormalImpulse;
    const FVector ImpactLocation = FVector(Hit.ImpactPoint);
    ApplyImpactDamage(OtherActor, ImpactLocation.IsNearlyZero() ? GetActorLocation() : ImpactLocation);
    Destroy();
}

void AHeroProjectile::ApplyImpactDamage(AActor* DirectHitActor, const FVector& ImpactLocation)
{
    if (DirectHitActor && DirectHitActor != InstigatorActor)
    {
        if (UHeroHealthComponent* Health = DirectHitActor->FindComponentByClass<UHeroHealthComponent>())
        {
            Health->ApplyDamage(Damage, InstigatorActor);
        }
    }

    if (ExplosionRadiusMeters <= 0.0f || !GetWorld())
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadiusMeters * 100.0f);
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    if (GetWorld()->OverlapMultiByObjectType(Overlaps, ImpactLocation, FQuat::Identity, ObjectQueryParams, Sphere))
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* Actor = Overlap.GetActor();
            if (!Actor || Actor == InstigatorActor || Actor == DirectHitActor)
            {
                continue;
            }

            if (UHeroHealthComponent* Health = Actor->FindComponentByClass<UHeroHealthComponent>())
            {
                const float Distance = FVector::Distance(ImpactLocation, Actor->GetActorLocation());
                const float Alpha = FMath::Clamp(1.0f - Distance / (ExplosionRadiusMeters * 100.0f), 0.15f, 1.0f);
                Health->ApplyDamage(Damage * Alpha, InstigatorActor);
            }
        }
    }
}

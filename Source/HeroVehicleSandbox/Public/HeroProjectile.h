#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeroProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroProjectile : public AActor
{
    GENERATED_BODY()

public:
    AHeroProjectile();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="Hero|Projectile")
    void ConfigureProjectile(float InDamage, float InExplosionRadiusMeters, float InImpactImpulse, AActor* InInstigatorActor);

    UFUNCTION(BlueprintCallable, Category="Hero|Projectile")
    void SetLaunchVelocity(const FVector& VelocityCmPerSecond);

protected:
    UFUNCTION()
    void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
    void ApplyImpactDamage(AActor* DirectHitActor, const FVector& ImpactLocation);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Projectile", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Projectile", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Projectile", meta=(AllowPrivateAccess="true"))
    float Damage = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Projectile", meta=(AllowPrivateAccess="true"))
    float ExplosionRadiusMeters = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Projectile", meta=(AllowPrivateAccess="true"))
    float ImpactImpulse = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Projectile", meta=(AllowPrivateAccess="true"))
    float LifeSeconds = 6.0f;

    UPROPERTY(Transient)
    TObjectPtr<AActor> InstigatorActor;
};

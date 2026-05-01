#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeroHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHeroHealthChangedSignature, float, Health, float, MaxHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHeroDeathSignature);

UCLASS(ClassGroup=(Hero), meta=(BlueprintSpawnableComponent))
class HEROVEHICLESANDBOX_API UHeroHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeroHealthComponent();

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    float ApplyDamage(float DamageAmount, AActor* DamageInstigator = nullptr);

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    float Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    void ResetHealth();

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    bool IsDead() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Health")
    float GetHealthRatio() const;

    UPROPERTY(BlueprintAssignable, Category="Hero|Health")
    FHeroHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Hero|Health")
    FHeroDeathSignature OnDeath;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Health", meta=(AllowPrivateAccess="true"))
    float MaxHealth = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Health", meta=(AllowPrivateAccess="true"))
    float Health = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Health", meta=(AllowPrivateAccess="true"))
    bool bDead = false;
};

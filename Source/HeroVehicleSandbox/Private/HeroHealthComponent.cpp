#include "HeroHealthComponent.h"

UHeroHealthComponent::UHeroHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHeroHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    Health = FMath::Clamp(Health <= 0.0f ? MaxHealth : Health, 0.0f, MaxHealth);
    bDead = Health <= 0.0f;
}

float UHeroHealthComponent::ApplyDamage(const float DamageAmount, AActor* DamageInstigator)
{
    (void)DamageInstigator;
    if (bDead || DamageAmount <= 0.0f)
    {
        return 0.0f;
    }

    const float OldHealth = Health;
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    const float Delta = Health - OldHealth;
    OnHealthChanged.Broadcast(Health, MaxHealth, Delta);

    if (Health <= 0.0f && !bDead)
    {
        bDead = true;
        OnDeath.Broadcast();
    }

    return -Delta;
}

float UHeroHealthComponent::Heal(const float HealAmount)
{
    if (bDead || HealAmount <= 0.0f)
    {
        return 0.0f;
    }

    const float OldHealth = Health;
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
    const float Delta = Health - OldHealth;
    if (!FMath::IsNearlyZero(Delta))
    {
        OnHealthChanged.Broadcast(Health, MaxHealth, Delta);
    }
    return Delta;
}

void UHeroHealthComponent::ResetHealth()
{
    bDead = false;
    const float OldHealth = Health;
    Health = MaxHealth;
    OnHealthChanged.Broadcast(Health, MaxHealth, Health - OldHealth);
}

float UHeroHealthComponent::GetHealth() const
{
    return Health;
}

float UHeroHealthComponent::GetMaxHealth() const
{
    return MaxHealth;
}

bool UHeroHealthComponent::IsDead() const
{
    return bDead;
}

float UHeroHealthComponent::GetHealthRatio() const
{
    return MaxHealth > KINDA_SMALL_NUMBER ? Health / MaxHealth : 0.0f;
}

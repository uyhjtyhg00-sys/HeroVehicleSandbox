#include "HeroAIController.h"

#include "EngineUtils.h"
#include "HeroBotCharacter.h"
#include "HeroCharacter.h"
#include "HeroGameModeBase.h"
#include "HeroHealthComponent.h"
#include "HeroTeamComponent.h"
#include "HeroWeaponComponent.h"

AHeroAIController::AHeroAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AHeroAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ReactionTimer = BotReactionTimeSeconds;
}

void AHeroAIController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (IsDead())
    {
        return;
    }

    UpdateKnownEnemies();
    UpdateKnownAllies();

    if (TryHealAlly())
    {
        UpdateStrafe(DeltaSeconds);
        return;
    }

    if (TryAttackEnemy(DeltaSeconds))
    {
        UpdateStrafe(DeltaSeconds);
        return;
    }

    if (TryMoveToObjective())
    {
        PerformIdleStrafe(DeltaSeconds);
        return;
    }

    PerformIdleStrafe(DeltaSeconds);
}

bool AHeroAIController::IsDead() const
{
    const AHeroCharacter* Hero = GetHeroPawn();
    const UHeroHealthComponent* Health = Hero ? Hero->GetHealthComponent() : nullptr;
    return !Hero || (Health && Health->IsDead());
}

void AHeroAIController::UpdateKnownEnemies()
{
    CurrentEnemy = nullptr;
    const AHeroCharacter* Hero = GetHeroPawn();
    if (!Hero || !GetWorld())
    {
        return;
    }

    const EHeroTeam MyTeam = GetMyTeam();
    float BestDistanceSq = TNumericLimits<float>::Max();

    for (TActorIterator<AHeroCharacter> It(GetWorld()); It; ++It)
    {
        AHeroCharacter* Other = *It;
        if (!Other || Other == Hero)
        {
            continue;
        }

        UHeroHealthComponent* OtherHealth = Other->GetHealthComponent();
        UHeroTeamComponent* OtherTeam = Other->GetTeamComponent();
        if (!OtherHealth || OtherHealth->IsDead() || !OtherTeam || !OtherTeam->IsEnemyTeam(MyTeam))
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(Hero->GetActorLocation(), Other->GetActorLocation());
        if (DistanceSq < BestDistanceSq && LineOfSightTo(Other))
        {
            BestDistanceSq = DistanceSq;
            CurrentEnemy = Other;
        }
    }
}

void AHeroAIController::UpdateKnownAllies()
{
    CurrentAllyToHeal = nullptr;
    const AHeroCharacter* Hero = GetHeroPawn();
    if (!Hero || !GetWorld())
    {
        return;
    }

    const EHeroTeam MyTeam = GetMyTeam();
    float LowestHealthRatio = AllyLowHealthThreshold;

    for (TActorIterator<AHeroCharacter> It(GetWorld()); It; ++It)
    {
        AHeroCharacter* Other = *It;
        if (!Other || Other == Hero)
        {
            continue;
        }

        UHeroHealthComponent* OtherHealth = Other->GetHealthComponent();
        UHeroTeamComponent* OtherTeam = Other->GetTeamComponent();
        if (!OtherHealth || OtherHealth->IsDead() || !OtherTeam || OtherTeam->GetTeam() != MyTeam)
        {
            continue;
        }

        const float Ratio = OtherHealth->GetHealthRatio();
        const float DistanceMeters = FVector::Distance(Hero->GetActorLocation(), Other->GetActorLocation()) * 0.01f;
        if (Ratio < LowestHealthRatio && DistanceMeters <= HealRangeMeters)
        {
            LowestHealthRatio = Ratio;
            CurrentAllyToHeal = Other;
        }
    }
}

bool AHeroAIController::TryHealAlly()
{
    AHeroBotCharacter* Bot = Cast<AHeroBotCharacter>(GetPawn());
    AHeroCharacter* Hero = GetHeroPawn();
    if (!Bot || Bot->GetBotRole() != EHeroBotRole::Support || !Hero || !CurrentAllyToHeal)
    {
        return false;
    }

    const FVector ToAlly = CurrentAllyToHeal->GetActorLocation() - Hero->GetActorLocation();
    SetControlRotation(ToAlly.Rotation());

    if (LineOfSightTo(CurrentAllyToHeal))
    {
        if (UHeroHealthComponent* Health = CurrentAllyToHeal->GetHealthComponent())
        {
            Health->Heal(HealingPerShot);
            return true;
        }
    }

    MoveToActor(CurrentAllyToHeal, 600.0f);
    return true;
}

bool AHeroAIController::TryAttackEnemy(const float DeltaSeconds)
{
    AHeroCharacter* Hero = GetHeroPawn();
    if (!Hero || !CurrentEnemy)
    {
        return false;
    }

    const FVector AimPoint = CurrentEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
    const FRotator DesiredRotation = (AimPoint - Hero->GetActorLocation()).Rotation();
    const FRotator NewRotation = FMath::RInterpTo(GetControlRotation(), DesiredRotation, DeltaSeconds, BotAimInterpSpeed);
    SetControlRotation(NewRotation);

    const float DistanceMeters = FVector::Distance(Hero->GetActorLocation(), CurrentEnemy->GetActorLocation()) * 0.01f;
    if (DistanceMeters > BotPreferredCombatDistanceMeters * 1.35f)
    {
        MoveToActor(CurrentEnemy, BotPreferredCombatDistanceMeters * 100.0f);
    }
    else
    {
        StopMovement();
    }

    ReactionTimer = FMath::Max(0.0f, ReactionTimer - DeltaSeconds);
    if (ReactionTimer <= 0.0f)
    {
        if (FMath::FRand() <= BotAccuracy)
        {
            Hero->GetWeaponComponent()->StartFire(nullptr);
        }
        else
        {
            Hero->GetWeaponComponent()->StopFire();
        }
        ReactionTimer = BotReactionTimeSeconds;
    }

    return true;
}

bool AHeroAIController::TryMoveToObjective()
{
    AHeroCharacter* Hero = GetHeroPawn();
    AHeroGameModeBase* HeroGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AHeroGameModeBase>() : nullptr;
    if (!Hero || !HeroGameMode)
    {
        return false;
    }

    AActor* Objective = HeroGameMode->GetPrimaryObjectiveForTeam(GetMyTeam());
    if (!Objective)
    {
        return false;
    }

    MoveToLocation(Objective->GetActorLocation(), 250.0f);
    return true;
}

void AHeroAIController::PerformIdleStrafe(const float DeltaSeconds)
{
    UpdateStrafe(DeltaSeconds);
}

void AHeroAIController::UpdateStrafe(const float DeltaSeconds)
{
    AHeroCharacter* Hero = GetHeroPawn();
    if (!Hero)
    {
        return;
    }

    StrafeTimer -= DeltaSeconds;
    if (StrafeTimer <= 0.0f)
    {
        CurrentStrafeDirection *= -1.0f;
        StrafeTimer = BotStrafeIntervalSeconds;
    }

    Hero->AddMovementInput(Hero->GetActorRightVector(), CurrentStrafeDirection * BotStrafeStrength);
}

AHeroCharacter* AHeroAIController::GetHeroPawn() const
{
    return Cast<AHeroCharacter>(GetPawn());
}

EHeroTeam AHeroAIController::GetMyTeam() const
{
    const AHeroCharacter* Hero = GetHeroPawn();
    const UHeroTeamComponent* Team = Hero ? Hero->GetTeamComponent() : nullptr;
    return Team ? Team->GetTeam() : EHeroTeam::None;
}

#include "HeroPayloadActor.h"

#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "HeroTeamComponent.h"

AHeroPayloadActor::AHeroPayloadActor()
{
    PrimaryActorTick.bCanEverTick = true;
    ContestVolume = CreateDefaultSubobject<USphereComponent>(TEXT("ContestVolume"));
    ContestVolume->SetSphereRadius(900.0f);
    ContestVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ContestVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    ContestVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetRootComponent(ContestVolume);
}

void AHeroPayloadActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdatePayloadProgress(DeltaSeconds);
}

void AHeroPayloadActor::SetPayloadPath(USplineComponent* InPath)
{
    PayloadPath = InPath;
    UpdateTransformFromPath();
}

void AHeroPayloadActor::UpdatePayloadProgress(const float DeltaSeconds)
{
    int32 Attackers = 0;
    int32 Defenders = 0;
    CountTeamsInRadius(Attackers, Defenders);

    if (PayloadProgress01 >= 1.0f)
    {
        ObjectiveState = EHeroObjectiveState::Completed;
        return;
    }

    if (Attackers > 0 && Defenders > 0)
    {
        ObjectiveState = EHeroObjectiveState::Contested;
        return;
    }

    if (Attackers > 0)
    {
        ObjectiveState = EHeroObjectiveState::Active;
        PayloadProgress01 = FMath::Clamp(PayloadProgress01 + PushSpeed01PerSecond * DeltaSeconds, 0.0f, 1.0f);
        UpdateTransformFromPath();
        return;
    }

    if (bAllowRollback && PayloadProgress01 > 0.0f)
    {
        PayloadProgress01 = FMath::Max(0.0f, PayloadProgress01 - RollbackSpeed01PerSecond * DeltaSeconds);
        ObjectiveState = EHeroObjectiveState::Active;
        UpdateTransformFromPath();
        return;
    }

    ObjectiveState = EHeroObjectiveState::Inactive;
}

float AHeroPayloadActor::GetPayloadProgress01() const
{
    return PayloadProgress01;
}

FVector AHeroPayloadActor::GetObjectiveLocationForTeam(const EHeroTeam Team) const
{
    (void)Team;
    return GetActorLocation();
}

EHeroObjectiveState AHeroPayloadActor::GetObjectiveState() const
{
    return ObjectiveState;
}

EHeroTeam AHeroPayloadActor::GetOwningTeam() const
{
    return PayloadProgress01 >= 1.0f ? AttackingTeam : EHeroTeam::None;
}

void AHeroPayloadActor::CountTeamsInRadius(int32& OutAttackers, int32& OutDefenders) const
{
    OutAttackers = 0;
    OutDefenders = 0;

    TArray<AActor*> OverlappingActors;
    ContestVolume->GetOverlappingActors(OverlappingActors);
    for (AActor* Actor : OverlappingActors)
    {
        const UHeroTeamComponent* TeamComponent = Actor ? Actor->FindComponentByClass<UHeroTeamComponent>() : nullptr;
        if (!TeamComponent)
        {
            continue;
        }

        if (TeamComponent->GetTeam() == AttackingTeam)
        {
            ++OutAttackers;
        }
        else if (TeamComponent->GetTeam() == DefendingTeam)
        {
            ++OutDefenders;
        }
    }
}

void AHeroPayloadActor::UpdateTransformFromPath()
{
    if (!PayloadPath)
    {
        return;
    }

    const float Distance = PayloadPath->GetSplineLength() * PayloadProgress01;
    const FVector Location = PayloadPath->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    const FRotator Rotation = PayloadPath->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    SetActorLocationAndRotation(Location, Rotation);
}

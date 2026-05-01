#include "HeroControlPointActor.h"

#include "Components/SphereComponent.h"
#include "HeroTeamComponent.h"

AHeroControlPointActor::AHeroControlPointActor()
{
    PrimaryActorTick.bCanEverTick = true;
    CaptureVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CaptureVolume"));
    CaptureVolume->SetSphereRadius(CaptureRadiusMeters * 100.0f);
    CaptureVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CaptureVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    CaptureVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetRootComponent(CaptureVolume);
}

void AHeroControlPointActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateCaptureProgress(DeltaSeconds);
}

void AHeroControlPointActor::UpdateCaptureProgress(const float DeltaSeconds)
{
    int32 TeamACount = 0;
    int32 TeamBCount = 0;
    CountTeamsInVolume(TeamACount, TeamBCount);

    if (TeamACount > 0 && TeamBCount > 0)
    {
        ObjectiveState = EHeroObjectiveState::Contested;
        return;
    }

    if (TeamACount <= 0 && TeamBCount <= 0)
    {
        ObjectiveState = OwningTeam == EHeroTeam::None ? EHeroObjectiveState::Inactive : EHeroObjectiveState::Captured;
        return;
    }

    ObjectiveState = EHeroObjectiveState::Active;
    if (TeamACount > 0)
    {
        TeamACaptureProgress = FMath::Clamp(TeamACaptureProgress + CaptureRatePerSecond * DeltaSeconds, 0.0f, 1.0f);
        TeamBCaptureProgress = FMath::Max(0.0f, TeamBCaptureProgress - CaptureRatePerSecond * DeltaSeconds * 0.5f);
        if (TeamACaptureProgress >= 1.0f)
        {
            OwningTeam = EHeroTeam::TeamA;
            ObjectiveState = EHeroObjectiveState::Captured;
        }
    }
    else if (TeamBCount > 0)
    {
        TeamBCaptureProgress = FMath::Clamp(TeamBCaptureProgress + CaptureRatePerSecond * DeltaSeconds, 0.0f, 1.0f);
        TeamACaptureProgress = FMath::Max(0.0f, TeamACaptureProgress - CaptureRatePerSecond * DeltaSeconds * 0.5f);
        if (TeamBCaptureProgress >= 1.0f)
        {
            OwningTeam = EHeroTeam::TeamB;
            ObjectiveState = EHeroObjectiveState::Captured;
        }
    }
}

FVector AHeroControlPointActor::GetObjectiveLocationForTeam(const EHeroTeam Team) const
{
    (void)Team;
    return GetActorLocation();
}

EHeroObjectiveState AHeroControlPointActor::GetObjectiveState() const
{
    return ObjectiveState;
}

EHeroTeam AHeroControlPointActor::GetOwningTeam() const
{
    return OwningTeam;
}

float AHeroControlPointActor::GetTeamACaptureProgress() const
{
    return TeamACaptureProgress;
}

float AHeroControlPointActor::GetTeamBCaptureProgress() const
{
    return TeamBCaptureProgress;
}

void AHeroControlPointActor::CountTeamsInVolume(int32& OutTeamA, int32& OutTeamB) const
{
    OutTeamA = 0;
    OutTeamB = 0;

    TArray<AActor*> OverlappingActors;
    CaptureVolume->GetOverlappingActors(OverlappingActors);
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor)
        {
            continue;
        }

        const UHeroTeamComponent* TeamComponent = Actor->FindComponentByClass<UHeroTeamComponent>();
        if (!TeamComponent)
        {
            continue;
        }

        if (TeamComponent->GetTeam() == EHeroTeam::TeamA)
        {
            ++OutTeamA;
        }
        else if (TeamComponent->GetTeam() == EHeroTeam::TeamB)
        {
            ++OutTeamB;
        }
    }
}

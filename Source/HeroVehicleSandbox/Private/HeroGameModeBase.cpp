#include "HeroGameModeBase.h"

#include "EngineUtils.h"
#include "HeroAIController.h"
#include "HeroBotCharacter.h"
#include "HeroCharacter.h"
#include "HeroControlPointActor.h"
#include "HeroObjectiveInterface.h"
#include "HeroPayloadActor.h"
#include "HeroPlayerController.h"

AHeroGameModeBase::AHeroGameModeBase()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = AHeroCharacter::StaticClass();
    PlayerControllerClass = AHeroPlayerController::StaticClass();
}

void AHeroGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    HeroMatchState = EHeroMatchState::Sandbox;
    CachePrimaryObjectives();
}

void AHeroGameModeBase::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    CachePrimaryObjectives();
}

void AHeroGameModeBase::ApplyCustomGameSettings(const FHeroCustomGameSettings& NewSettings)
{
    CustomGameSettings = NewSettings;
    HeroMatchState = CustomGameSettings.GameModeType == EHeroGameModeType::Sandbox ? EHeroMatchState::Sandbox : EHeroMatchState::CombatTest;
}

const FHeroCustomGameSettings& AHeroGameModeBase::GetCustomGameSettings() const
{
    return CustomGameSettings;
}

AActor* AHeroGameModeBase::GetPrimaryObjectiveForTeam(const EHeroTeam Team) const
{
    (void)Team;
    if (CustomGameSettings.GameModeType == EHeroGameModeType::Escort && PrimaryPayload)
    {
        return PrimaryPayload;
    }
    if (CustomGameSettings.GameModeType == EHeroGameModeType::Control && PrimaryControlPoint)
    {
        return PrimaryControlPoint;
    }
    return PrimaryControlPoint ? Cast<AActor>(PrimaryControlPoint) : Cast<AActor>(PrimaryPayload);
}

EHeroObjectiveState AHeroGameModeBase::GetPrimaryObjectiveState() const
{
    if (const AActor* Objective = GetPrimaryObjectiveForTeam(EHeroTeam::TeamA))
    {
        if (const IHeroObjectiveInterface* Interface = Cast<IHeroObjectiveInterface>(Objective))
        {
            return Interface->GetObjectiveState();
        }
    }
    return EHeroObjectiveState::Inactive;
}

float AHeroGameModeBase::GetPrimaryObjectiveProgress01() const
{
    if (CustomGameSettings.GameModeType == EHeroGameModeType::Escort && PrimaryPayload)
    {
        return PrimaryPayload->GetPayloadProgress01();
    }
    if (PrimaryControlPoint)
    {
        return FMath::Max(PrimaryControlPoint->GetTeamACaptureProgress(), PrimaryControlPoint->GetTeamBCaptureProgress());
    }
    return 0.0f;
}

EHeroMatchState AHeroGameModeBase::GetHeroMatchState() const
{
    return HeroMatchState;
}

void AHeroGameModeBase::CachePrimaryObjectives()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (!PrimaryControlPoint)
    {
        for (TActorIterator<AHeroControlPointActor> It(World); It; ++It)
        {
            PrimaryControlPoint = *It;
            break;
        }
    }

    if (!PrimaryPayload)
    {
        for (TActorIterator<AHeroPayloadActor> It(World); It; ++It)
        {
            PrimaryPayload = *It;
            break;
        }
    }
}

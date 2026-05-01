#include "HeroTeamComponent.h"

UHeroTeamComponent::UHeroTeamComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHeroTeamComponent::SetTeam(const EHeroTeam NewTeam)
{
    Team = NewTeam;
}

EHeroTeam UHeroTeamComponent::GetTeam() const
{
    return Team;
}

bool UHeroTeamComponent::IsEnemyTeam(const EHeroTeam OtherTeam) const
{
    return Team != EHeroTeam::None && OtherTeam != EHeroTeam::None && Team != OtherTeam;
}

#include "HeroVehicleModeComponent.h"

UHeroVehicleModeComponent::UHeroVehicleModeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHeroVehicleModeComponent::ToggleVehicleMode()
{
    if (PlayerMode == EHeroPlayerMode::Human)
    {
        EnterVehicleMode();
    }
    else if (PlayerMode == EHeroPlayerMode::Vehicle)
    {
        ExitVehicleMode();
    }
}

void UHeroVehicleModeComponent::EnterVehicleMode()
{
    SetPlayerMode(EHeroPlayerMode::Vehicle);
}

void UHeroVehicleModeComponent::ExitVehicleMode()
{
    SetPlayerMode(EHeroPlayerMode::Human);
}

EHeroPlayerMode UHeroVehicleModeComponent::GetPlayerMode() const
{
    return PlayerMode;
}

void UHeroVehicleModeComponent::SetPlayerMode(const EHeroPlayerMode NewMode)
{
    if (PlayerMode == NewMode)
    {
        return;
    }

    PlayerMode = NewMode;
    OnPlayerModeChanged.Broadcast(PlayerMode);
}

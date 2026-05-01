#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeroTypes.h"
#include "HeroVehicleModeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeroPlayerModeChangedSignature, EHeroPlayerMode, NewMode);

UCLASS(ClassGroup=(Hero), meta=(BlueprintSpawnableComponent))
class HEROVEHICLESANDBOX_API UHeroVehicleModeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeroVehicleModeComponent();

    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle Mode")
    void ToggleVehicleMode();

    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle Mode")
    void EnterVehicleMode();

    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle Mode")
    void ExitVehicleMode();

    UFUNCTION(BlueprintCallable, Category="Hero|Vehicle Mode")
    EHeroPlayerMode GetPlayerMode() const;

    UPROPERTY(BlueprintAssignable, Category="Hero|Vehicle Mode")
    FHeroPlayerModeChangedSignature OnPlayerModeChanged;

private:
    void SetPlayerMode(EHeroPlayerMode NewMode);

private:
    UPROPERTY(BlueprintReadOnly, Category="Hero|Vehicle Mode", meta=(AllowPrivateAccess="true"))
    EHeroPlayerMode PlayerMode = EHeroPlayerMode::Human;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Vehicle Mode", meta=(AllowPrivateAccess="true"))
    bool bVehicleAbilitiesReserved = true;
};

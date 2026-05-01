#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeroCustomGameSettings.h"
#include "HeroGameModeBase.generated.h"

class AHeroControlPointActor;
class AHeroPayloadActor;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHeroGameModeBase();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    void ApplyCustomGameSettings(const FHeroCustomGameSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category="Hero|Custom Game")
    const FHeroCustomGameSettings& GetCustomGameSettings() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    AActor* GetPrimaryObjectiveForTeam(EHeroTeam Team) const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    EHeroObjectiveState GetPrimaryObjectiveState() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    float GetPrimaryObjectiveProgress01() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Match")
    EHeroMatchState GetHeroMatchState() const;

private:
    void CachePrimaryObjectives();

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Custom Game", meta=(AllowPrivateAccess="true"))
    FHeroCustomGameSettings CustomGameSettings;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Match", meta=(AllowPrivateAccess="true"))
    EHeroMatchState HeroMatchState = EHeroMatchState::Boot;

    UPROPERTY(Transient)
    TObjectPtr<AHeroControlPointActor> PrimaryControlPoint;

    UPROPERTY(Transient)
    TObjectPtr<AHeroPayloadActor> PrimaryPayload;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeroObjectiveInterface.h"
#include "HeroPayloadActor.generated.h"

class USphereComponent;
class USplineComponent;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroPayloadActor : public AActor, public IHeroObjectiveInterface
{
    GENERATED_BODY()

public:
    AHeroPayloadActor();
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Hero|Payload")
    void SetPayloadPath(USplineComponent* InPath);

    UFUNCTION(BlueprintCallable, Category="Hero|Payload")
    void UpdatePayloadProgress(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category="Hero|Payload")
    float GetPayloadProgress01() const;

    virtual FVector GetObjectiveLocationForTeam(EHeroTeam Team) const override;
    virtual EHeroObjectiveState GetObjectiveState() const override;
    virtual EHeroTeam GetOwningTeam() const override;

private:
    void CountTeamsInRadius(int32& OutAttackers, int32& OutDefenders) const;
    void UpdateTransformFromPath();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USphereComponent> ContestVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USplineComponent> PayloadPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    EHeroTeam AttackingTeam = EHeroTeam::TeamA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    EHeroTeam DefendingTeam = EHeroTeam::TeamB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    float PushSpeed01PerSecond = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    float RollbackSpeed01PerSecond = 0.008f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    bool bAllowRollback = true;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    float PayloadProgress01 = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Payload", meta=(AllowPrivateAccess="true"))
    EHeroObjectiveState ObjectiveState = EHeroObjectiveState::Inactive;
};

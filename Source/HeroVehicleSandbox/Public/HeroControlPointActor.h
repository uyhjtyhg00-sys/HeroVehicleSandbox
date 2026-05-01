#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeroObjectiveInterface.h"
#include "HeroControlPointActor.generated.h"

class USphereComponent;

UCLASS()
class HEROVEHICLESANDBOX_API AHeroControlPointActor : public AActor, public IHeroObjectiveInterface
{
    GENERATED_BODY()

public:
    AHeroControlPointActor();
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    void UpdateCaptureProgress(float DeltaSeconds);

    virtual FVector GetObjectiveLocationForTeam(EHeroTeam Team) const override;
    virtual EHeroObjectiveState GetObjectiveState() const override;
    virtual EHeroTeam GetOwningTeam() const override;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    float GetTeamACaptureProgress() const;

    UFUNCTION(BlueprintCallable, Category="Hero|Objective")
    float GetTeamBCaptureProgress() const;

private:
    void CountTeamsInVolume(int32& OutTeamA, int32& OutTeamB) const;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USphereComponent> CaptureVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    float CaptureRatePerSecond = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    float CaptureRadiusMeters = 9.0f;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    float TeamACaptureProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    float TeamBCaptureProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    EHeroTeam OwningTeam = EHeroTeam::None;

    UPROPERTY(BlueprintReadOnly, Category="Hero|Objective", meta=(AllowPrivateAccess="true"))
    EHeroObjectiveState ObjectiveState = EHeroObjectiveState::Inactive;
};
